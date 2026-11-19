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
private:
    juce::Rectangle<float> plot() const;
    juce::Point<int> cellAt(juce::Point<float>, bool clamp) const;
    void stroke(juce::Point<int>);
    RaBubanjProcessor& processor;
    juce::Point<int> last { -1, -1 };
    int editValue = 0;
    bool drawing = false, changed = false;
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
private:
    void timerCallback() override { refresh(); }
    RaBubanjProcessor& processor;
    StepGrid grid;
    juce::TextButton enabled { "SEQ ON" };
    std::array<juce::TextButton, 3> velocity;
    MidiPatternDrag midiDrag;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enabledAttachment;
};
// Child window keeps keyboard focus within the VST3 view so unhandled Space reaches the host.
class SequencerWindow final : public juce::DocumentWindow
{
public:
    explicit SequencerWindow(RaBubanjProcessor&);
    void closeButtonPressed() override { setVisible(false); }
    bool keyPressed(const juce::KeyPress& key) override
    { return key.getKeyCode() == juce::KeyPress::spaceKey ? false : DocumentWindow::keyPressed(key); }
};
