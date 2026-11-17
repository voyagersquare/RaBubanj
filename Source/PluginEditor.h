#pragma once
#include "PluginProcessor.h"

class RaBubanjEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit RaBubanjEditor(RaBubanjProcessor&);
    ~RaBubanjEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    struct Channel
    {
        juce::TextButton pad, mute;
        juce::Slider gain, pan;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment, panAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> muteAttachment;
        uint32_t lastHit = 0;
        float flash = 0;
    };
    RaBubanjProcessor& processor;
    juce::LookAndFeel_V4 look;
    juce::TooltipWindow tooltip { this, 600 };
    std::array<Channel, rabubanj::drumCount> channels;
    juce::Slider master, room;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterAttachment, roomAttachment;
    juce::Rectangle<int> padArea, mixerArea, footerArea;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RaBubanjEditor)
};
