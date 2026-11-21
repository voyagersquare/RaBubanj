#pragma once
#include "PluginProcessor.h"
#include "UiTheme.h"

class NoteTimingEditor final : public juce::Component
{
public:
    NoteTimingEditor(RaBubanjProcessor&, int pattern, int row, int step);
    void resized() override;
    void paint(juce::Graphics&) override;
    void refresh();
    int patternIndex;
private:
    RaBubanjProcessor& processor;
    int row, step;
    juce::Slider timing;
    juce::TextButton close { "X" };
};
class StepGrid final : public juce::Component, private juce::Timer
{
public:
    explicit StepGrid(RaBubanjProcessor&);
    ~StepGrid() override { stopTimer(); }
    void refresh();
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
    void timerCallback() override;
    void openTimingEditor();
    RaBubanjProcessor& processor;
    juce::Point<int> last { -1, -1 };
    int selectedPad = 0;
    int editPattern = 0;
    juce::Point<int> pressedCell { -1, -1 };
    juce::Point<float> downPosition;
    juce::Time downTime;
    bool drawing = false, erasing = false, leftPending = false, longOpened = false, changed = false;
    std::unique_ptr<NoteTimingEditor> timingEditor;
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
// A dedicated page inside the plugin, with explicit controls independent of native title bars.
class SequencerWindow final : public juce::Component
{
public:
    explicit SequencerWindow(RaBubanjProcessor&);
    void closeButtonPressed() { if (onClose) onClose(); else setVisible(false); }
    std::function<void()> onClose;
    void visibilityChanged() override { content.refresh(); }
    juce::Button* getCloseButton() noexcept { return &close; }
    void resized() override;
    void paint(juce::Graphics&) override;
    bool keyPressed(const juce::KeyPress&) override { return false; }
private:
    SequencerPanel content;
    juce::TextButton close { "X" };
};
