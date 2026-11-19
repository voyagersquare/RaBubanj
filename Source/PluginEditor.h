#pragma once
#include "EqPanel.h"

class MixerStrip final : public juce::Component, public juce::FileDragAndDropTarget, public juce::TextDragAndDropTarget
{
public:
    MixerStrip(RaBubanjProcessor&, int bus);
    ~MixerStrip() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void refresh();
    bool isInterestedInFileDrag(const juce::StringArray&) override;
    void fileDragEnter(const juce::StringArray&, int, int) override;
    void fileDragExit(const juce::StringArray&) override;
    void filesDropped(const juce::StringArray&, int, int) override;
    bool isInterestedInTextDrag(const juce::String&) override;
    void textDragEnter(const juce::String&, int, int) override;
    void textDragExit(const juce::String&) override;
    void textDropped(const juce::String&, int, int) override;
    std::function<void(int)> onSelect;
private:
    RaBubanjProcessor& processor;
    const int bus;
    juce::String channelName;
    juce::TextButton select, mute { "M" }, solo { "S" };
    juce::TextButton preview { "PLAY" }, clear { "x" };
    juce::Label sampleName;
    uint32_t previewUntil = 0;
    bool dragging = false;
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
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider::SliderStyle, juce::Slider&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override
    {
        return juce::Font(juce::FontOptions(14.0f));
    }
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
    bool keyPressed(const juce::KeyPress&) override;
    bool keyStateChanged(bool) override;
    void focusLost(FocusChangeType) override;
private:
    void timerCallback() override;
    RaBubanjProcessor& processor;
    MixerLookAndFeel look;
    juce::TooltipWindow tooltip { this, 600 };
    EqPanel channelEq, globalEq;
    std::array<std::unique_ptr<MixerStrip>, rabubanj::eqBusCount> strips;
    void bindChannelControls();
    juce::Slider size, damping, width, pitch, drive, compression;
    int colourBus = -1;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> pitchAttachment, driveAttachment, compAttachment;
    juce::TextButton keyboardMode, midiMode;
    std::array<bool, 8> heldKeys {};
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sizeAttachment, dampingAttachment, widthAttachment;
    juce::Rectangle<int> eqArea, mixerArea, reverbArea, reverbControlsArea, reservedArea, footerArea;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RaBubanjEditor)
};
