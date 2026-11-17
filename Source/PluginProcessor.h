#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "DrumEngine.h"

class RaBubanjProcessor final : public juce::AudioProcessor
{
public:
    RaBubanjProcessor();
    ~RaBubanjProcessor() override = default;
    void prepareToPlay(double, int) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "RaBubanj"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 6.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "RaBubanj Studio"; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    void audition(int drum) noexcept;
    float getPeak() const noexcept { return peak.load(); }
    uint32_t getHitCount(int drum) const noexcept { return hits[static_cast<size_t>(drum)].load(); }
    juce::AudioProcessorValueTreeState parameters;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
    void hit(int note, float velocity) noexcept;
    rabubanj::DrumEngine engine;
    juce::Reverb reverb;
    juce::SmoothedValue<float> masterGain;
    std::array<std::atomic<float>*, rabubanj::drumCount> gainParams {}, panParams {}, muteParams {};
    std::atomic<float>* masterParam = nullptr;
    std::atomic<float>* roomParam = nullptr;
    std::array<std::atomic<uint32_t>, rabubanj::drumCount> hits {};
    std::atomic<float> peak { 0 };
    juce::AbstractFifo padFifo { 64 };
    std::array<int, 64> padQueue {};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RaBubanjProcessor)
};
