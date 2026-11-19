#include "PluginProcessor.h"
#include <cstdlib>
#include <iostream>
#include <thread>
static void require(bool ok, const char* message)
{ if (!ok) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); } }
static void set(RaBubanjProcessor& p, const juce::String& id, float value)
{ auto* parameter = p.parameters.getParameter(id); parameter->setValueNotifyingHost(parameter->convertTo0to1(value)); }
int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    for (const auto rate : { 44100.0, 48000.0, 96000.0 })
    {
        rabubanj::ChannelColour neutral; neutral.set(0, 0); neutral.prepare(rate);
        for (int i = 0; i < 8192; ++i)
        {
            float l = static_cast<float>(std::sin(i*.173)*.7), r = -l*.5f; const auto a = l, b = r;
            neutral.process(l, r); require(l == a && r == b, "zero drive and compression must be bit-exact bypass");
        }
        rabubanj::ChannelColour compressor; compressor.set(0, 1); compressor.prepare(rate);
        float l = 0, r = 0;
        for (int i = 0; i < static_cast<int>(rate*.3); ++i) { l = .5f; r = .25f; compressor.process(l, r); }
        require(std::abs(compressor.gainReductionDb()-7.9863) < .05, "settled compressor must match -18 dBFS / 3:1 gain reduction");
        require(std::abs(l/r-2) < .00001f && l < .21f && l > .19f, "linked compression must preserve stereo balance");
        const auto before = compressor.gainReductionDb();
        for (int i = 0; i < static_cast<int>(rate*.05); ++i) { l = r = 0; compressor.process(l, r); }
        require(compressor.gainReductionDb() < before && compressor.gainReductionDb() > before*.5, "opto-style release must decay gradually");
        rabubanj::ChannelColour saturation; saturation.set(1, 0); saturation.prepare(rate);
        double fundamental = 0, third = 0;
        for (int i = 0; i < static_cast<int>(rate); ++i)
        {
            const auto phase = i*juce::MathConstants<double>::twoPi*997/rate;
            l = r = static_cast<float>(std::sin(phase)*.5); saturation.process(l, r);
            require(std::isfinite(l) && std::abs(l) < 1.01f, "saturation must stay finite and bounded");
            fundamental += l*std::sin(phase); third += l*std::sin(phase*3);
        }
        require(std::abs(third/fundamental) > .1, "drive must add audible harmonic content");
    }
    // Exact pitch / source-speed relationship and playing-voice automation.
    rabubanj::CustomSample source; source.sampleRate = 48000; source.left.resize(48000);
    for (int i = 0; i < 48000; ++i) source.left[static_cast<size_t>(i)] = static_cast<float>(std::sin(i*juce::MathConstants<double>::twoPi*200/48000));
    rabubanj::DrumEngine engine; engine.prepare(48000); engine.setCustomSample(0, &source, true); engine.setPitch(0, 12); engine.triggerDrum(0, 1);
    std::array<std::array<float, 1024>, 8> left {}, right {}; std::array<float*, 8> lp {}, rp {};
    for (size_t i = 0; i < 8; ++i) { lp[i] = left[i].data(); rp[i] = right[i].data(); }
    engine.renderStereoStems(lp, rp, 1024);
    for (int i = 0; i < 1024; ++i) require(std::abs(left[0][static_cast<size_t>(i)]-source.left[static_cast<size_t>(i*2)]) < .000001f, "+12 semitones must double the playback rate");
    engine.setPitch(0, -24); left[0].fill(0); right[0].fill(0); engine.renderStereoStems(lp, rp, 1024);
    for (float value : left[0]) require(std::isfinite(value), "live pitch changes must remain finite");
    engine.reset();
    require(source.voiceReferences.load() == 0, "pitched voices must release their source");
    rabubanj::SpectrumTap tap; rabubanj::SpectrumAnalyser spectrum;
    std::array<float, 1024> sine {}, inverted {};
    for (int block = 0; block < 12; ++block)
    {
        for (int i = 0; i < 1024; ++i) { sine[static_cast<size_t>(i)] = static_cast<float>(.5*std::sin((block*1024+i)*juce::MathConstants<double>::twoPi*996.09375/48000)); inverted[static_cast<size_t>(i)] = -sine[static_cast<size_t>(i)]; }
        tap.push(sine.data(), inverted.data(), 1024); spectrum.update(tap);
    }
    const auto& bins = spectrum.getBins(); const auto maximum = std::max_element(bins.begin(), bins.end());
    require(std::abs(std::distance(bins.begin(), maximum)*48000.0/4096-996.09375) < 12 && std::abs(*maximum+6.0206f) < .2f, "FFT must measure frequency and dBFS without anti-phase cancellation");
    sine.fill(0);
    for (int i = 0; i < 50; ++i) { tap.push(sine.data(), sine.data(), 1024); spectrum.update(tap); }
    require(*std::max_element(bins.begin(), bins.end()) < -80, "spectrum must decay after silence");
    // SPSC publication under simultaneous producer and GUI-consumer activity.
    std::atomic<bool> running { true }; std::thread writer([&] { for (int i = 0; i < 3000; ++i) tap.push(sine.data(), sine.data(), 1024); running.store(false); });
    while (running.load()) spectrum.update(tap);
    writer.join();
    RaBubanjProcessor p; p.setRateAndBufferSizeDetails(48000, 512); p.prepareToPlay(48000, 512); p.setAnalyserActive(true);
    juce::AudioBuffer<float> audio(2, 512); juce::MidiBuffer midi; midi.addEvent(juce::MidiMessage::noteOn(1, 36, .9f), 0);
    rabubanj::SpectrumAnalyser channel, master;
    for (int i = 0; i < 20; ++i) { p.processBlock(audio, midi); channel.update(p.getSpectrumTap(0)); master.update(p.getSpectrumTap(9)); }
    require(*std::max_element(channel.getBins().begin(), channel.getBins().end()) > -40
        && *std::max_element(master.getBins().begin(), master.getBins().end()) > -45, "channel and master taps must carry actual processor audio");
    set(p, "pitch0", -9.5f); set(p, "drive0", .8f); set(p, "comp0", .6f);
    juce::MemoryBlock state; p.getStateInformation(state); RaBubanjProcessor restored; restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    for (const auto* id : { "pitch0", "drive0", "comp0" }) require(std::abs(p.parameters.getRawParameterValue(id)->load()-restored.parameters.getRawParameterValue(id)->load()) < .001f, "channel colour parameters must recall");
    std::cout << "Neutral bypass, stereo opto-style compression, saturation harmonics, pitch rate, FFT calibration/decay, concurrent taps and state recall passed.\n";
}
