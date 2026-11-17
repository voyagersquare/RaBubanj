#include "PluginProcessor.h"
#include <cstdlib>
#include <iostream>

static void require(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
static void setParameter(RaBubanjProcessor& processor, const char* id, float value)
{
    auto* parameter = processor.parameters.getParameter(id);
    parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
}
int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    RaBubanjProcessor p;
    p.setRateAndBufferSizeDetails(48000, 512);
    p.prepareToPlay(48000, 512);
    juce::AudioBuffer<float> audio(2, 512);
    juce::MidiBuffer midi;
    p.processBlock(audio, midi);
    require(audio.getMagnitude(0, 512) == 0, "idle processor must be silent");
    midi.addEvent(juce::MidiMessage::noteOn(1, 38, .9f), 123);
    p.processBlock(audio, midi);
    require(audio.getMagnitude(0, 123) == 0, "MIDI hit must not precede its sample offset");
    require(audio.getMagnitude(124, 388) > .01f, "MIDI hit must sound after its offset");
    require(midi.isEmpty(), "instrument must consume MIDI");
    require(p.getHitCount(1) == 1, "MIDI must update pad indicator");

    midi.addEvent(juce::MidiMessage::allSoundOff(1), 77);
    p.processBlock(audio, midi);
    require(audio.getMagnitude(0, 77) > 0, "audio before panic must survive");
    require(audio.getMagnitude(77, 435) == 0, "panic must silence voices and room at the exact offset");
    p.processBlock(audio, midi);
    require(audio.getMagnitude(0, 512) == 0, "panic must leave no room tail");

    p.audition(0);
    p.processBlock(audio, midi);
    require(audio.getMagnitude(0, 512) > .01f && p.getHitCount(0) == 1, "UI queue must trigger audio and indicator");
    setParameter(p, "gain1", -12.3f);
    setParameter(p, "pan1", .45f);
    setParameter(p, "mute2", 1);
    setParameter(p, "master", -15.2f);
    setParameter(p, "room", .67f);
    juce::MemoryBlock state;
    p.getStateInformation(state);
    RaBubanjProcessor restored;
    restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    for (const auto* id : { "gain1", "pan1", "mute2", "master", "room" })
        require(std::abs(p.parameters.getRawParameterValue(id)->load() - restored.parameters.getRawParameterValue(id)->load()) < .001f,
            "DAW project state must recall mixer parameters");

    p.releaseResources();
    p.prepareToPlay(96000, 512);
    for (int hit = 0; hit < 512; ++hit)
        midi.addEvent(juce::MidiMessage::noteOn(1, rabubanj::notes[static_cast<size_t>(hit % 8)], 1.0f), hit);
    p.processBlock(audio, midi);
    for (int c = 0; c < 2; ++c)
        for (int s = 0; s < 512; ++s)
            require(std::isfinite(audio.getSample(c, s)) && std::abs(audio.getSample(c, s)) <= 1,
                "dense MIDI must remain finite and bounded");
    std::cout << "Processor MIDI offsets, panic, preview, state recall and dense input checks passed.\n";
}
