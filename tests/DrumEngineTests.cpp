#include "DrumEngine.h"
#include <cstdlib>
#include <iostream>
#include <numeric>

static void require(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
static double energy(const std::vector<float>& signal)
{
    return std::inner_product(signal.begin(), signal.end(), signal.begin(), 0.0);
}
int main()
{
    for (double rate : { 44100.0, 48000.0, 96000.0 })
    {
        rabubanj::DrumEngine engine;
        engine.prepare(rate);
        rabubanj::Mix mix;
        const int length = static_cast<int>(rate * 3);
        std::vector<float> left(static_cast<size_t>(length)), right(left.size());
        engine.render(left.data(), right.data(), length, mix);
        require(energy(left) == 0, "idle engine must be silent");
        for (int note : rabubanj::notes)
        {
            engine.reset();
            std::fill(left.begin(), left.end(), 0);
            std::fill(right.begin(), right.end(), 0);
            engine.trigger(note, .9f);
            // Nonuniform blocks must preserve voice position and one-shot tails.
            for (int offset = 0; offset < length;)
            {
                const int block = std::min(137, length - offset);
                engine.render(left.data() + offset, right.data() + offset, block, mix);
                offset += block;
            }
            require(energy(left) > .01, "each mapped drum must sound");
            require(engine.activeVoiceCount() == 0, "one-shot voices must finish");
            for (float sample : left) require(std::isfinite(sample), "output must be finite");
        }

        auto kickEnergy = [&](float velocity)
        {
            engine.reset();
            std::fill(left.begin(), left.end(), 0);
            std::fill(right.begin(), right.end(), 0);
            engine.trigger(36, velocity);
            engine.render(left.data(), right.data(), length, mix);
            return energy(left);
        };
        require(kickEnergy(1.0f) > kickEnergy(.2f) * 8, "velocity must affect dynamics");
        mix.pan[0] = -1;
        kickEnergy(.8f);
        require(energy(left) > .01 && energy(right) < 1e-8, "hard-left pan must isolate left channel");
        mix.mute[0] = true;
        require(kickEnergy(.8f) == 0, "mute must silence drum");
        mix = {};

        engine.reset();
        engine.trigger(46, 1);
        engine.render(left.data(), right.data(), 512, mix);
        engine.trigger(42, 1);
        engine.render(left.data(), right.data(), static_cast<int>(rate * .21), mix);
        require(engine.activeVoiceCount() == 0, "closed hat must choke open hat");

        engine.reset();
        engine.trigger(60, 1);
        engine.trigger(36, 0);
        require(engine.activeVoiceCount() == 0, "unknown notes and zero velocity must be ignored");
        for (int i = 0; i < 1000; ++i) engine.trigger(38, .7f);
        require(engine.activeVoiceCount() == 64, "voice stealing must bound polyphony");
        engine.reset();
        require(engine.activeVoiceCount() == 0, "panic must clear voices");

        std::vector<float> reference(2048), second(2048), scratch(2048);
        engine.trigger(38, .8f);
        engine.render(reference.data(), scratch.data(), 2048, mix);
        engine.reset();
        std::fill(scratch.begin(), scratch.end(), 0);
        engine.trigger(38, .8f);
        engine.render(second.data(), scratch.data(), 2048, mix);
        require(reference == second, "reset must produce deterministic first variation");
        std::fill(second.begin(), second.end(), 0);
        std::fill(scratch.begin(), scratch.end(), 0);
        // Let the first hit end, preserving round-robin counters.
        engine.render(left.data(), right.data(), length, mix);
        engine.trigger(38, .8f);
        engine.render(second.data(), scratch.data(), 2048, mix);
        require(reference != second, "round robin must vary repeated hits");
    }
    require(rabubanj::drumForNote(35) == 0 && rabubanj::drumForNote(44) == 2
        && rabubanj::drumForNote(59) == 7, "GM aliases must map correctly");
    std::cout << "All drum engine checks passed (44.1 / 48 / 96 kHz).\n";
}
