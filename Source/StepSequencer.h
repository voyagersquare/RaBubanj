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
    static constexpr int rows = 8, steps = 32;
    using Snapshot = std::array<uint8_t, rows*steps>;
    inline static constexpr std::array<int, 5> velocities { 0, 32, 48, 88, 127 };
    uint8_t get(int row, int step) const noexcept { return valid(row, step) ? cells[static_cast<size_t>(row*steps+step)].load() : 0; }
    bool set(int row, int step, int strength) noexcept
    {
        if (!valid(row, step)) return false;
        const auto value = static_cast<uint8_t>(std::clamp(strength, 0, 4));
        return cells[static_cast<size_t>(row*steps+step)].exchange(value) != value;
    }
    int cycle(int row, int step) noexcept
    {
        if (!valid(row, step)) return 0;
        auto& cell = cells[static_cast<size_t>(row*steps+step)]; auto current = cell.load(); uint8_t next;
        do { next = current <= 1 ? 4 : static_cast<uint8_t>(current-1); } while (!cell.compare_exchange_weak(current, next));
        return next;
    }
    Snapshot snapshot() const noexcept
    { Snapshot result {}; for (size_t i = 0; i < result.size(); ++i) result[i] = cells[i].load(); return result; }
    void replace(const Snapshot& pattern) noexcept
    { for (size_t i = 0; i < pattern.size(); ++i) cells[i].store(std::min<uint8_t>(4, pattern[i])); }
private:
    static bool valid(int row, int step) noexcept { return row >= 0 && row < rows && step >= 0 && step < steps; }
    std::array<std::atomic<uint8_t>, rows*steps> cells {};
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
    SequenceCursor(SequenceTransport transport, double sampleRate, int samples) noexcept : host(transport), count(samples)
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
    static int wrap(int64_t step) noexcept { return static_cast<int>((step%32+32)%32); }
    void beginSegment() noexcept
    {
        boundarySample = loop ? segmentSample+(host.loopEnd-segmentPpq)*samplesPerBeat : static_cast<double>(count);
        segmentLimit = std::min(static_cast<double>(count), std::floor(boundarySample+.5+1e-8));
        nextStep = static_cast<int64_t>(std::ceil((segmentPpq+(segmentMinimum-.5-segmentSample)/samplesPerBeat)*4-1e-9));
    }
    SequenceTransport host;
    int count = 0;
    bool valid = false, loop = false;
    double samplesPerBeat = 0, segmentSample = 0, segmentPpq = 0, segmentMinimum = 0, segmentLimit = 0, boundarySample = 0;
    int64_t nextStep = 0;
};
}
