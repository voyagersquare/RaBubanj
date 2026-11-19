#include "StepSequencer.h"
#include <cstdlib>
#include <iostream>
#include <vector>
static void require(bool ok, const char* text) { if (!ok) { std::cerr << "FAIL: " << text << '\n'; std::exit(1); } }
int main()
{
    rabubanj::StepPattern pattern;
    require(pattern.get(0, 0) == 0 && pattern.set(0, 0, 3) && !pattern.set(0, 0, 3), "pattern edits must be bounded and idempotent");
    require(!pattern.set(-1, 0, 1) && !pattern.set(0, 32, 1), "invalid cells must be ignored");
    pattern.set(7, 31, 100); require(pattern.snapshot()[255] == 3, "strength must clamp to three levels");
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
    std::cout << "32-step phase, three velocities, fractional tempo/block boundaries, loop wrapping, seek, pre-roll and stopped transport passed.\n";
}
