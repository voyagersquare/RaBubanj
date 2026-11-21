#pragma once
#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
namespace rabubanj
{
class StepPattern
{
public:
    static constexpr int rows = 8, steps = 128, defaultSteps = 32;
    static int normaliseLength(int length) noexcept { return length == 64 || length == 128 ? length : defaultSteps; }
    int getLength() const noexcept { return length.load(); }
    bool setLength(int value) noexcept { const auto next = normaliseLength(value); return length.exchange(next) != next; }
    using Snapshot = std::array<uint8_t, rows*steps>;
    struct Cell { uint8_t strength = 0; int8_t timing = 0; };
    using TimedSnapshot = std::array<Cell, rows*steps>;
    inline static constexpr std::array<int, 5> velocities { 0, 32, 48, 88, 127 };
    uint8_t get(int row, int step) const noexcept { return valid(row, step) ? static_cast<uint8_t>(cells[index(row, step)].load() & 7u) : 0; }
    int getTiming(int row, int step) const noexcept { return valid(row, step) ? decodeTiming(cells[index(row, step)].load()) : 0; }
    bool set(int row, int step, int strength) noexcept
    {
        if (!valid(row, step)) return false;
        auto& cell = cells[index(row, step)]; auto old = cell.load(); uint16_t next;
        do { next = strength <= 0 ? 0 : static_cast<uint16_t>((old & ~7u) | static_cast<unsigned>(std::clamp(strength, 0, 4))); } while (!cell.compare_exchange_weak(old, next));
        return old != next;
    }
    bool setTiming(int row, int step, int percent) noexcept
    {
        if (!valid(row, step)) return false;
        auto& cell = cells[index(row, step)]; auto old = cell.load(); uint16_t next;
        do { next = pack(static_cast<int>(old & 7u), percent); } while (!cell.compare_exchange_weak(old, next));
        return old != next;
    }
    int cycle(int row, int step) noexcept
    {
        if (!valid(row, step)) return 0;
        auto& cell = cells[index(row, step)]; auto old = cell.load(); uint16_t next;
        do { const auto strength = old & 7u; next = static_cast<uint16_t>((old & ~7u) | (strength <= 1 ? 4u : strength-1)); } while (!cell.compare_exchange_weak(old, next));
        return static_cast<int>(next & 7u);
    }
    Snapshot snapshot() const noexcept
    { Snapshot result {}; for (size_t i = 0; i < result.size(); ++i) result[i] = static_cast<uint8_t>(cells[i].load() & 7u); return result; }
    TimedSnapshot timedSnapshot() const noexcept
    {
        TimedSnapshot result {};
        for (size_t i = 0; i < result.size(); ++i) { const auto value = cells[i].load(); result[i] = { static_cast<uint8_t>(value & 7u), static_cast<int8_t>(decodeTiming(value)) }; }
        return result;
    }
    void replace(const Snapshot& pattern) noexcept
    { for (size_t i = 0; i < pattern.size(); ++i) cells[i].store(pack(pattern[i], 0)); }
    void replaceTimed(const TimedSnapshot& pattern) noexcept
    { for (size_t i = 0; i < pattern.size(); ++i) cells[i].store(pack(pattern[i].strength, pattern[i].timing)); }
private:
    static uint16_t pack(int strength, int timing) noexcept
    { return static_cast<uint16_t>(static_cast<unsigned>(std::clamp(strength, 0, 4)) | ((static_cast<unsigned>(std::clamp(timing, -50, 50)) & 127u) << 3)); }
    static int decodeTiming(uint16_t value) noexcept { const auto raw = static_cast<int>(value >> 3); return raw >= 64 ? raw-128 : raw; }
    static size_t index(int row, int step) noexcept { return static_cast<size_t>(row*steps+step); }
    static bool valid(int row, int step) noexcept { return row >= 0 && row < rows && step >= 0 && step < steps; }
    // Strength and timing of each cell are published atomically together.
    std::array<std::atomic<uint16_t>, rows*steps> cells {};
    std::atomic<int> length { defaultSteps };
};
struct SequenceTransport
{
    bool playing = false, valid = false, looping = false;
    double ppq = 0, bpm = 120, loopStart = 0, loopEnd = 0;
};
// A bounded, allocation-free iterator. Each event is quantised to its nearest audio sample.
// The half-sample look-behind preserves fractional-tempo events at block boundaries.
class SequenceCursor
{
public:
    struct Event { int offset = 0, step = 0; };
    SequenceCursor(SequenceTransport transport, double sampleRate, int samples, int length = StepPattern::defaultSteps) noexcept
        : host(transport), count(samples), stepCount(StepPattern::normaliseLength(length))
    {
        valid = host.valid && host.playing && std::isfinite(host.ppq) && std::abs(host.ppq) < 1e12
            && std::isfinite(host.bpm) && host.bpm >= 1 && host.bpm <= 1000 && sampleRate >= 8000 && samples > 0;
        if (!valid) return;
        samplesPerBeat = sampleRate*60/host.bpm;
        loop = host.looping && std::isfinite(host.loopStart) && std::isfinite(host.loopEnd)
            && std::abs(host.loopStart) < 1e12 && std::abs(host.loopEnd) < 1e12
            && (host.loopEnd-host.loopStart)*samplesPerBeat >= 1;
        if (loop && host.ppq >= host.loopEnd) host.ppq = host.loopStart+std::fmod(host.ppq-host.loopEnd, host.loopEnd-host.loopStart);
        segmentPpq = host.ppq; beginSegment();
    }
    bool next(Event& result) noexcept
    {
        while (valid)
        {
            const auto raw = segmentSample+(static_cast<double>(nextStep)*.25-segmentPpq)*samplesPerBeat;
            const auto rounded = std::floor(raw+.5+1e-8);
            if (rounded >= segmentLimit)
            {
                if (!loop || segmentLimit >= count) { valid = false; break; }
                segmentSample = boundarySample; segmentPpq = host.loopStart; segmentMinimum = segmentLimit;
                beginSegment(); continue;
            }
            ++nextStep;
            if (rounded < segmentMinimum) continue;
            result.offset = static_cast<int>(rounded); result.step = wrap(nextStep-1); return true;
        }
        return false;
    }
    int currentStep() const noexcept
    {
        if (!host.valid || !host.playing || samplesPerBeat <= 0) return -1;
        auto ppq = host.ppq+static_cast<double>(count-1)/samplesPerBeat;
        if (loop && ppq >= host.loopEnd) ppq = host.loopStart+std::fmod(ppq-host.loopEnd, host.loopEnd-host.loopStart);
        return wrap(static_cast<int64_t>(std::floor(ppq*4+1e-9)));
    }
private:
    int wrap(int64_t step) const noexcept { return static_cast<int>((step%stepCount+stepCount)%stepCount); }
    void beginSegment() noexcept
    {
        boundarySample = loop ? segmentSample+(host.loopEnd-segmentPpq)*samplesPerBeat : static_cast<double>(count);
        segmentLimit = std::min(static_cast<double>(count), std::floor(boundarySample+.5+1e-8));
        nextStep = static_cast<int64_t>(std::ceil((segmentPpq+(segmentMinimum-.5-segmentSample)/samplesPerBeat)*4-1e-9));
    }
    SequenceTransport host;
    int count = 0, stepCount = StepPattern::defaultSteps;
    bool valid = false, loop = false;
    double samplesPerBeat = 0, segmentSample = 0, segmentPpq = 0, segmentMinimum = 0, segmentLimit = 0, boundarySample = 0;
    int64_t nextStep = 0;
};

// Each active cell contributes one regular stream and at most one host-loop edge stream.
// Negative timing crosses only the start edge; positive timing crosses only the end edge.
// A fixed heap orders individual hits, including negative offsets across pattern/host-loop edges.
class NoteSequenceCursor
{
public:
    struct Event { int offset = 0, row = 0, step = 0, strength = 0; };
    NoteSequenceCursor(SequenceTransport transport, double rate, int samples, const StepPattern::TimedSnapshot& cells, int length = StepPattern::defaultSteps) noexcept
        : host(transport), pattern(cells), count(samples), stepCount(StepPattern::normaliseLength(length)), patternBeats(stepCount*.25)
    {
        valid = host.valid && host.playing && std::isfinite(host.ppq) && std::abs(host.ppq) < 1e12
            && std::isfinite(host.bpm) && host.bpm >= 1 && host.bpm <= 1000 && rate >= 8000 && samples > 0;
        if (!valid) return;
        spq = rate*60/host.bpm;
        loop = host.looping && std::isfinite(host.loopStart) && std::isfinite(host.loopEnd)
            && std::abs(host.loopStart) < 1e12 && std::abs(host.loopEnd) < 1e12 && (host.loopEnd-host.loopStart)*spq >= 1;
        if (loop && host.ppq >= host.loopEnd) host.ppq = host.loopStart+std::fmod(host.ppq-host.loopEnd, host.loopEnd-host.loopStart);
        preRoll = loop && host.ppq < host.loopStart;
        build(host.ppq-.5/spq);
    }
    bool next(Event& event) noexcept
    {
        while (valid && used > 0)
        {
            if (preRoll && heap[0].beat >= host.loopStart)
            { preRoll = false; build(host.loopStart); continue; }
            const auto offset = std::floor((heap[0].beat-host.ppq)*spq+.5+1e-8);
            if (offset >= count) return false;
            std::pop_heap(heap.begin(), heap.begin()+used, Later{}); auto node = heap[static_cast<size_t>(--used)];
            const auto current = node;
            node.beat += node.increment;
            if (node.regularLoop && node.beat-node.cycle >= node.limit-1e-9)
            { node.cycle += host.loopEnd-host.loopStart; node.beat = node.first+node.cycle; }
            heap[static_cast<size_t>(used++)] = node; std::push_heap(heap.begin(), heap.begin()+used, Later{});
            if (offset < 0) continue;
            event = { static_cast<int>(offset), current.cell/StepPattern::steps, current.cell%StepPattern::steps, pattern[static_cast<size_t>(current.cell)].strength }; return true;
        }
        return false;
    }
private:
    struct Node { double beat = 0, increment = 8, first = 0, limit = 0, cycle = 0; int cell = 0; bool regularLoop = false; };
    struct Later { bool operator()(const Node& a, const Node& b) const noexcept { return a.beat > b.beat || (a.beat == b.beat && a.cell > b.cell); } };
    static double wrap(double value, double length) noexcept { const auto mod = std::fmod(value, length); return mod < 0 ? mod+length : mod; }
    void add(Node node) noexcept { heap[static_cast<size_t>(used++)] = node; }
    void build(double threshold) noexcept
    {
        used = 0; const auto length = host.loopEnd-host.loopStart;
        for (int row = 0; row < StepPattern::rows; ++row) for (int step = 0; step < stepCount; ++step)
        {
            const auto cell = row*StepPattern::steps+step;
            const auto& value = pattern[static_cast<size_t>(cell)]; if (value.strength == 0) continue;
            const auto base = static_cast<double>(step)*.25, shift = static_cast<double>(value.timing)*.0025, shifted = base+shift;
            if (!loop || preRoll)
            { Node n; n.cell = cell; n.increment = patternBeats; n.beat = shifted+patternBeats*std::ceil((threshold-shifted)/patternBeats-1e-10); add(n); continue; }
            const auto low = std::max(host.loopStart, host.loopStart+shift), high = std::min(host.loopEnd, host.loopEnd+shift);
            const auto first = shifted+patternBeats*std::ceil((low-shifted)/patternBeats-1e-10);
            if (first < high-1e-9)
            {
                Node n; n.cell = cell; n.increment = patternBeats; n.first = first; n.limit = high; n.regularLoop = true;
                n.beat = first+patternBeats*std::max(0.0, std::ceil((threshold-first)/patternBeats-1e-10));
                if (n.beat >= high-1e-9) { n.cycle = length; n.beat = first+length; }
                // A rounded event immediately before the loop start belongs to sample zero.
                const auto last = first+patternBeats*(std::ceil((high-first)/patternBeats-1e-10)-1);
                if (last-length >= threshold && last-length < n.beat) { n.cycle = -length; n.beat = last-length; }
                add(n);
            }
            const auto firstNominal = base+patternBeats*std::ceil((host.loopStart-base)/patternBeats-1e-10);
            const auto lastNominal = base+patternBeats*(std::ceil((host.loopEnd-base)/patternBeats-1e-10)-1);
            for (int edge = 0; edge < 2; ++edge)
            {
                const auto nominal = edge == 0 ? firstNominal : lastNominal;
                if (nominal < host.loopStart-1e-9 || nominal >= host.loopEnd-1e-9) continue;
                const auto moved = nominal+shift;
                if ((edge == 0 && moved >= host.loopStart) || (edge == 1 && moved < host.loopEnd)) continue;
                const auto phase = host.loopStart+wrap(moved-host.loopStart, length);
                Node n; n.cell = cell; n.increment = length; n.beat = phase+length*std::ceil((threshold-phase)/length-1e-10); add(n);
            }
        }
        std::make_heap(heap.begin(), heap.begin()+used, Later{});
    }
    SequenceTransport host;
    const StepPattern::TimedSnapshot& pattern;
    std::array<Node, StepPattern::rows*StepPattern::steps*2> heap {};
    int used = 0, count = 0, stepCount = StepPattern::defaultSteps;
    double spq = 0, patternBeats = 8;
    bool valid = false, loop = false, preRoll = false;
};
}
