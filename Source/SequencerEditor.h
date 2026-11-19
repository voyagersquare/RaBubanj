#pragma once
#include "PluginProcessor.h"
#include "UiTheme.h"

class StepGrid final : public juce::Component
{
public:
    explicit StepGrid(RaBubanjProcessor&);
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    juce::Rectangle<float> cellBounds(int row, int step) const;
    juce::Rectangle<float> padBounds(int row) const;
    int getSelectedPad() const noexcept { return selectedPad; }
private:
    juce::Rectangle<float> plot() const;
    juce::Point<int> cellAt(juce::Point<float>, bool clamp) const;
    void stroke(juce::Point<int>);
    RaBubanjProcessor& processor;
    juce::Point<int> last { -1, -1 };
    int selectedPad = 0;
    int editPattern = 0;
    bool drawing = false, changed = false;
};
class PadWaveform final : public juce::Component
{
public:
    explicit PadWaveform(RaBubanjProcessor& p) : processor(p) { setName("Selected pad waveform"); setWantsKeyboardFocus(false); }
    void update(int pad, bool active);
    void paint(juce::Graphics&) override;
    float getDisplayedPeak() const noexcept;
private:
    RaBubanjProcessor& processor;
    std::array<rabubanj::WaveformTap::Frame, 2048> history {};
    int position = 0, selectedPad = 0;
    uint32_t token = 0;
};
class MidiPatternDrag final : public juce::TextButton
{
public:
    explicit MidiPatternDrag(RaBubanjProcessor&);
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
private:
    RaBubanjProcessor& processor;
    bool started = false;
};
class SequencerPanel final : public juce::Component, private juce::Timer
{
public:
    explicit SequencerPanel(RaBubanjProcessor&);
    ~SequencerPanel() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void refresh();
    void visibilityChanged() override;
private:
    void timerCallback() override { refresh(); }
    RaBubanjProcessor& processor;
    StepGrid grid;
    PadWaveform waveform;
    juce::TextButton enabled { "SEQ ON" };
    juce::ComboBox patternChoice;
    MidiPatternDrag midiDrag;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enabledAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> patternAttachment;
};
// Explicit child-panel controls avoid native title-button recreation by host/look-and-feel changes.
class SequencerWindow final : public juce::Component
{
public:
    explicit SequencerWindow(RaBubanjProcessor&);
    void closeButtonPressed() { setVisible(false); }
    void visibilityChanged() override { content.refresh(); }
    juce::Button* getCloseButton() noexcept { return &close; }
    void resized() override;
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    bool keyPressed(const juce::KeyPress&) override { return false; }
private:
    SequencerPanel content;
    juce::TextButton close { "X" };
    juce::ComponentDragger dragger;
    juce::ComponentBoundsConstrainer constrainer;
};
