#pragma once
#include "EqPanel.h"

class MixerStrip final : public juce::Component
{
public:
    MixerStrip(RaBubanjProcessor&, int bus);
    ~MixerStrip() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void refresh();
    std::function<void(int)> onSelect;
private:
    RaBubanjProcessor& processor;
    const int bus;
    juce::String channelName;
    juce::TextButton select, eq { "EQ" }, mute { "M" }, solo { "S" }, audition;
    juce::Slider gain, pan, send;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment, panAttachment, sendAttachment;
    std::unique_ptr<ButtonAttachment> muteAttachment, soloAttachment;
    juce::Rectangle<int> faderArea, meterArea;
    uint32_t lastHit = 0;
    float flash = 0;
};

class MixerLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    MixerLookAndFeel();
    void drawLinearSlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider::SliderStyle, juce::Slider&) override;
    juce::Label* createSliderTextBox(juce::Slider& s) override
    {
        auto* label = juce::LookAndFeel_V4::createSliderTextBox(s);
        label->setFont(juce::FontOptions(11.0f)); label->setBorderSize(juce::BorderSize<int>(0, 1, 0, 1)); return label;
    }
    juce::Font getTextButtonFont(juce::TextButton&, int height) override { return juce::Font(juce::FontOptions(juce::jmin(12.0f, static_cast<float>(height)*.52f))); }
};

class RaBubanjEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit RaBubanjEditor(RaBubanjProcessor&);
    ~RaBubanjEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void selectChannel(int bus);
private:
    void timerCallback() override;
    RaBubanjProcessor& processor;
    MixerLookAndFeel look;
    juce::TooltipWindow tooltip { this, 600 };
    EqPanel channelEq, globalEq;
    std::array<std::unique_ptr<MixerStrip>, rabubanj::eqBusCount> strips;
    juce::Slider size, damping, width;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sizeAttachment, dampingAttachment, widthAttachment;
    juce::Rectangle<int> eqArea, mixerArea, reverbArea, footerArea;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RaBubanjEditor)
};
