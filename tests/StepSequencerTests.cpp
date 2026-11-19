#include "StepSequencer.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <tuple>
static void require(bool ok, const char* text) { if (!ok) { std::cerr << "FAIL: " << text << '\n'; std::exit(1); } }
int main()
{
    rabubanj::StepPattern pattern;
    require(pattern.get(0, 0) == 0 && pattern.set(0, 0, 3) && !pattern.set(0, 0, 3), "pattern edits must be bounded and idempotent");
    require(!pattern.set(-1, 0, 1) && !pattern.set(0, 32, 1), "invalid cells must be ignored");
    pattern.set(7, 31, 100); require(pattern.snapshot()[255] == 4, "strength must clamp to four levels");
    pattern.set(1, 1, 0);
    for (int expected : { 4, 3, 2, 1, 4 }) require(pattern.cycle(1, 1) == expected, "left-click velocity cycle must wrap without erasing");
    require(pattern.cycle(-1, 0) == 0, "invalid cycle coordinates must be ignored");
    for (double rate : { 44100.0, 48000.0, 96000.0 }) for (double bpm : { 61.1, 93.0, 127.35, 199.99 }) for (int block : { 37, 511, 1024 })
    {
        const auto spq = rate*60/bpm; const auto total = static_cast<int>(std::floor(spq*16));
        std::vector<int> events; int step = 0;
        for (int offset = 0; offset < total; offset += block)
        {
            rabubanj::SequenceTransport host; host.valid = host.playing = true; host.ppq = offset/spq; host.bpm = bpm;
            rabubanj::SequenceCursor cursor(host, rate, std::min(block, total-offset)); rabubanj::SequenceCursor::Event event;
            while (cursor.next(event))
            {
                require(event.step == step%32, "step order must wrap exactly at 32");
                require(event.offset >= 0 && event.offset < std::min(block, total-offset), "scheduled offsets must fit the audio buffer");
                events.push_back(offset+event.offset); ++step;
            }
        }
        require(events.size() == 64, "two pattern cycles must contain 64 boundaries with no missing/duplicate steps");
        for (size_t i = 0; i < events.size(); ++i) require(events[i] == static_cast<int>(std::floor(static_cast<double>(i)*spq*.25+.5+1e-8)), "fractional BPM scheduling must be sample accurate across blocks");
    }
    rabubanj::SequenceTransport host; host.valid = host.playing = host.looping = true; host.bpm = 120; host.ppq = 3.75; host.loopStart = 0; host.loopEnd = 4;
    rabubanj::SequenceCursor loop(host, 48000, 13000); rabubanj::SequenceCursor::Event e;
    for (const auto expected : { std::pair<int, int>(0, 15), { 6000, 0 }, { 12000, 1 } })
        require(loop.next(e) && e.offset == expected.first && e.step == expected.second, "host loop wrapping inside a block must follow the loop start");
    require(!loop.next(e), "loop cursor must stop at the buffer boundary");
    host.looping = false; host.ppq = 2.1;
    rabubanj::SequenceCursor seek(host, 48000, 6000); require(seek.next(e) && e.offset == 3600 && e.step == 9, "starting between steps must align to the next host grid boundary");
    host.ppq = -.25; rabubanj::SequenceCursor negative(host, 48000, 7000); require(negative.next(e) && e.offset == 0 && e.step == 31 && negative.next(e) && e.offset == 6000 && e.step == 0, "negative pre-roll must retain modulo phase");
    host.playing = false; rabubanj::SequenceCursor stopped(host, 48000, 1024); require(!stopped.next(e) && stopped.currentStep() == -1, "stopped DAW must not schedule notes");
    host.valid = false; host.playing = true; rabubanj::SequenceCursor missing(host, 48000, 1024); require(!missing.next(e), "missing host position must not run a free clock");
    pattern.setTiming(1, 1, -99); require(pattern.getTiming(1, 1) == -50 && pattern.get(1, 1) == 4, "early timing must clamp and preserve velocity");
    pattern.cycle(1, 1); require(pattern.getTiming(1, 1) == -50, "velocity cycling must preserve note timing");
    pattern.setTiming(1, 1, 99); require(pattern.getTiming(1, 1) == 50, "late timing must clamp");
    pattern.set(1, 1, 0); require(pattern.getTiming(1, 1) == 0, "erasing a note must also clear its timing");
    require(!pattern.setTiming(-1, 0, 20), "invalid timing cells must be ignored");
    using Hit = std::tuple<int, int, int>;
    pattern.replace({});
    for (int row = 0; row < 8; ++row) for (int step = 0; step < 32; ++step) { pattern.set(row, step, row%4+1); pattern.setTiming(row, step, (step*17+row*11)%101-50); }
    const auto timed = pattern.timedSnapshot();
    for (double rate : { 44100.0, 48000.0, 96000.0 }) for (double bpm : { 61.1, 93.0, 127.35, 199.99 }) for (int block : { 37, 511, 1024 })
    {
        const auto spq = rate*60/bpm; const auto total = static_cast<int>(std::floor(spq*16)); std::vector<Hit> actual, expected;
        for (int offset = 0; offset < total; offset += block)
        {
            rabubanj::SequenceTransport t; t.valid = t.playing = true; t.ppq = offset/spq; t.bpm = bpm;
            rabubanj::NoteSequenceCursor cursor(t, rate, std::min(block, total-offset), timed); rabubanj::NoteSequenceCursor::Event hit;
            while (cursor.next(hit)) actual.emplace_back(offset+hit.offset, hit.row, hit.step);
        }
        for (int cycle = -1; cycle <= 2; ++cycle) for (int cell = 0; cell < 256; ++cell)
        {
            const auto beat = cycle*8+static_cast<double>(cell%32)*.25+static_cast<double>(timed[static_cast<size_t>(cell)].timing)*.0025;
            const auto sample = static_cast<int>(std::floor(beat*spq+.5+1e-8)); if (sample >= 0 && sample < total) expected.emplace_back(sample, cell/32, cell%32);
        }
        std::sort(actual.begin(), actual.end()); std::sort(expected.begin(), expected.end());
        require(actual == expected, "all per-note early/late offsets must remain sample-accurate across fractional-BPM block and pattern edges");
    }
    for (const auto bounds : { std::pair<double, double>(0, 4), { .13, 7.91 }, { 2, 22 }, { -.2, .17 } }) for (int block : { 37, 511, 1024 })
    {
        const double bpm = 127.35, rate = 48000, spq = rate*60/bpm, length = bounds.second-bounds.first, start = bounds.first+length*.37;
        const auto total = static_cast<int>(std::floor(length*spq*2)); std::vector<Hit> actual, expected;
        for (int offset = 0; offset < total; offset += block)
        {
            rabubanj::SequenceTransport t; t.valid = t.playing = t.looping = true; t.bpm = bpm; t.loopStart = bounds.first; t.loopEnd = bounds.second;
            t.ppq = bounds.first+std::fmod(start-bounds.first+offset/spq, length);
            rabubanj::NoteSequenceCursor cursor(t, rate, std::min(block, total-offset), timed); rabubanj::NoteSequenceCursor::Event note;
            while (cursor.next(note)) actual.emplace_back(offset+note.offset, note.row, note.step);
        }
        for (int cell = 0; cell < 256; ++cell) for (int cycle = -2; cycle <= 5; ++cycle)
        {
            const auto nominal = cycle*8+static_cast<double>(cell%32)*.25; if (nominal < bounds.first || nominal >= bounds.second) continue;
            auto phase = std::fmod(nominal+timed[static_cast<size_t>(cell)].timing*.0025-bounds.first, length); if (phase < 0) phase += length;
            for (int repeat = -1; repeat <= 3; ++repeat)
            { const auto sample = static_cast<int>(std::floor((bounds.first+phase+repeat*length-start)*spq+.5+1e-8)); if (sample >= 0 && sample < total) expected.emplace_back(sample, cell/32, cell%32); }
        }
        std::sort(actual.begin(), actual.end()); std::sort(expected.begin(), expected.end());
        require(actual == expected, "microtiming must follow short, long, negative and non-grid host loops across arbitrary audio blocks");
    }
    pattern.replace({}); pattern.set(0, 0, 4); pattern.setTiming(0, 0, -50); pattern.set(1, 15, 3); pattern.setTiming(1, 15, 50); pattern.set(2, 1, 2);
    auto loopPattern = pattern.timedSnapshot(); host.valid = host.playing = host.looping = true; host.ppq = 3.75; host.bpm = 120; host.loopStart = 0; host.loopEnd = 4;
    rabubanj::NoteSequenceCursor shiftedLoop(host, 48000, 13000, loopPattern); rabubanj::NoteSequenceCursor::Event hit;
    require(shiftedLoop.next(hit) && hit.offset == 3000 && hit.row == 0, "early first note must anticipate the next host-loop start");
    require(shiftedLoop.next(hit) && hit.offset == 3000 && hit.row == 1, "late snare must be ordered independently of early kick at the same sample");
    require(shiftedLoop.next(hit) && hit.offset == 12000 && hit.row == 2 && !shiftedLoop.next(hit), "subsequent loop notes must stay aligned");
    pattern.replace({}); pattern.set(0, 0, 4); pattern.setTiming(0, 0, 50); loopPattern = pattern.timedSnapshot(); host.ppq = 0; host.loopEnd = .1;
    rabubanj::NoteSequenceCursor tinyLoop(host, 48000, 7200, loopPattern);
    for (int sample : { 600, 3000, 5400 }) require(tinyLoop.next(hit) && hit.offset == sample, "late notes must wrap even inside short non-grid host loops");
    require(!tinyLoop.next(hit), "loop iterator must end at block boundary");
    host.looping = false; host.ppq = -.2; rabubanj::NoteSequenceCursor pre(host, 48000, 10000, loopPattern);
    require(pre.next(hit) && hit.offset == 7800, "negative pre-roll must preserve shifted hit timing");
    host.playing = false; rabubanj::NoteSequenceCursor muted(host, 48000, 10000, loopPattern); require(!muted.next(hit), "stopped transport must not emit shifted hits");
    std::cout << "32-step phase, four velocities, fractional tempo/block boundaries, loop wrapping, seek, pre-roll and stopped transport passed.\n";
}
