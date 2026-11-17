#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "DrumEngine.h"
#include "ParametricEq.h"

class RaBubanjProcessor final : public juce::AudioProcessor
{
public:
    RaBubanjProcessor();
    ~RaBubanjProcessor() override = default;
    void prepareToPlay(double, int) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    using juce::AudioProcessor::processBlock;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "RaBubanj"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 10.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "RaBubanj Studio"; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    void audition(int drum) noexcept;
    float getPeak(int bus = rabubanj::masterBus) const noexcept { return peaks[static_cast<size_t>(bus)].load(); }
    uint32_t getHitCount(int drum) const noexcept { return hits[static_cast<size_t>(drum)].load(); }
    int getSelectedChannel() const noexcept { return selectedChannel.load(); }
    void selectChannel(int bus) noexcept { selectedChannel.store(juce::jlimit(0, rabubanj::reverbBus, bus)); }
    rabubanj::EqSettings getEqSettings(int bus) const noexcept;
    double getDisplaySampleRate() const noexcept { return displaySampleRate.load(); }
    juce::AudioProcessorValueTreeState parameters;

private:
    struct EqParameters
    {
        struct Band { std::atomic<float>* frequency = nullptr; std::atomic<float>* gain = nullptr;
            std::atomic<float>* q = nullptr; std::atomic<float>* type = nullptr; std::atomic<float>* enabled = nullptr; };
        std::array<Band, rabubanj::eqBandCount> bands;
        std::atomic<float>* bypass = nullptr;
    };
    static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
    void hit(int note, float velocity) noexcept;
    void resetAudio() noexcept;
    rabubanj::DrumEngine engine;
    juce::Reverb reverb;
    juce::SmoothedValue<float> masterGain, returnGain;
    std::array<juce::SmoothedValue<float>, rabubanj::drumCount> gains, pans, sends, dryLevels;
    std::array<std::atomic<float>*, rabubanj::drumCount> gainParams {}, panParams {}, muteParams {}, sendParams {}, presetParams {};
    std::array<std::atomic<float>*, rabubanj::drumCount + 1> soloParams {};
    std::array<EqParameters, rabubanj::eqBusCount> eqParams;
    std::array<rabubanj::ParametricEq, rabubanj::eqBusCount> equalisers;
    std::atomic<float>* masterParam = nullptr; std::atomic<float>* roomParam = nullptr;
    std::atomic<float>* reverbMuteParam = nullptr; std::atomic<float>* masterMuteParam = nullptr;
    std::atomic<float>* sizeParam = nullptr; std::atomic<float>* dampingParam = nullptr; std::atomic<float>* widthParam = nullptr;
    std::array<std::atomic<uint32_t>, rabubanj::drumCount> hits {};
    std::array<std::atomic<float>, rabubanj::eqBusCount> peaks {};
    std::atomic<int> selectedChannel { 0 };
    std::atomic<double> displaySampleRate { 44100 };
    static constexpr int chunkSize = 256;
    std::array<std::array<float, chunkSize>, rabubanj::drumCount> stems {};
    std::array<float, chunkSize> wetLeft {}, wetRight {};
    juce::AbstractFifo padFifo { 64 };
    std::array<int, 64> padQueue {};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RaBubanjProcessor)
};
