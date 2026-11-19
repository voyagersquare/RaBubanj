#include "SequencerEditor.h"
using namespace rabubanj::ui;
StepGrid::StepGrid(RaBubanjProcessor& p) : processor(p) { setName("32 step grid"); setWantsKeyboardFocus(false); }
juce::Rectangle<float> StepGrid::plot() const { return getLocalBounds().toFloat().reduced(8, 4).withTrimmedRight(86).withTrimmedTop(27); }
juce::Rectangle<float> StepGrid::cellBounds(int row, int step) const
{
    const auto r = plot(); return { r.getX()+static_cast<float>(step)*r.getWidth()/32, r.getY()+static_cast<float>(row)*r.getHeight()/8, r.getWidth()/32, r.getHeight()/8 };
}
juce::Point<int> StepGrid::cellAt(juce::Point<float> p, bool clamp) const
{
    const auto r = plot(); if (!clamp && !r.contains(p)) return { -1, -1 };
    if (r.getWidth() <= 0 || r.getHeight() <= 0) return { -1, -1 };
    return { juce::jlimit(0, 31, static_cast<int>(std::floor((p.x-r.getX())*32/r.getWidth()))), juce::jlimit(0, 7, static_cast<int>(std::floor((p.y-r.getY())*8/r.getHeight()))) };
}
void StepGrid::stroke(juce::Point<int> cell)
{
    if (cell.x < 0 || cell.y < 0) return;
    auto previous = last.x < 0 ? cell : last;
    const auto dx = std::abs(cell.x-previous.x), dy = -std::abs(cell.y-previous.y);
    const int sx = previous.x < cell.x ? 1 : -1, sy = previous.y < cell.y ? 1 : -1;
    auto error = dx+dy;
    for (;;)
    {
        changed = processor.getPattern().set(previous.y, previous.x, editValue) || changed;
        if (previous == cell) break;
        const auto twice = error*2;
        if (twice >= dy) { error += dy; previous.x += sx; }
        if (twice <= dx) { error += dx; previous.y += sy; }
    }
    last = cell; repaint();
}
void StepGrid::mouseDown(const juce::MouseEvent& e)
{
    if (!e.mods.isLeftButtonDown() && !e.mods.isRightButtonDown()) return;
    last = { -1, -1 }; changed = false;
    drawing = cellAt(e.position, false).x >= 0;
    editValue = e.mods.isRightButtonDown() ? processor.getSequenceBrush() : 0;
    if (drawing) stroke(cellAt(e.position, false));
}
void StepGrid::mouseDrag(const juce::MouseEvent& e) { if (drawing) stroke(cellAt(e.position, true)); }
void StepGrid::mouseUp(const juce::MouseEvent&)
{ if (changed) processor.sequenceEdited(); drawing = changed = false; last = { -1, -1 }; }
void StepGrid::paint(juce::Graphics& g)
{
    insetPanel(g, getLocalBounds().toFloat()); const auto r = plot(); const auto playing = processor.getPlayingStep();
    for (int step = 0; step < 32; ++step)
    {
        const auto box = cellBounds(0, step);
        if (step == playing) { g.setColour(gold.withAlpha(.18f)); g.fillRect(box.withHeight(r.getHeight()).expanded(0, 1)); }
        text(g, juce::String(step+1), { juce::roundToInt(box.getX()), 5, juce::roundToInt(box.getWidth()), 22 }, 11, step == playing ? gold : ink, juce::Justification::centred, true);
        for (int row = 0; row < 8; ++row)
        {
            auto cell = cellBounds(row, step).reduced(2, 3);
            const auto value = processor.getPattern().get(row, step);
            g.setColour(step/4%2 == 0 ? juce::Colour(0xff3d382a) : juce::Colour(0xff302e24)); g.fillRoundedRectangle(cell, 2);
            if (value > 0)
            {
                const std::array<juce::Colour, 3> colours { juce::Colour(0xff8c7d50), juce::Colour(0xffbc9e49), juce::Colour(0xffffd46b) };
                g.setColour(colours[static_cast<size_t>(value-1)]);
                auto fill = cell.reduced(2); fill.removeFromTop(fill.getHeight()*(1-static_cast<float>(value)/3)); g.fillRoundedRectangle(fill, 1);
            }
            g.setColour(step == playing ? gold : juce::Colour(0xff6e5b2b)); g.drawRoundedRectangle(cell, 2, step == playing ? 1.4f : .7f);
        }
    }
    for (int row = 0; row < 8; ++row)
    {
        const auto box = cellBounds(row, 31);
        text(g, rabubanj::padNames[static_cast<size_t>(row)], { juce::roundToInt(r.getRight())+8, juce::roundToInt(box.getY()), 74, juce::roundToInt(box.getHeight()) }, 14, ink, juce::Justification::centred, true);
    }
}
MidiPatternDrag::MidiPatternDrag(RaBubanjProcessor& p) : TextButton("DRAG MIDI"), processor(p)
{
    setName("Drag sequence MIDI"); setWantsKeyboardFocus(false);
    setTooltip("Drag this two-bar MIDI pattern into the DAW. Switch SEQ OFF when playing the exported MIDI clip to avoid doubled hits.");
}
void MidiPatternDrag::mouseDown(const juce::MouseEvent& e) { started = false; TextButton::mouseDown(e); }
void MidiPatternDrag::mouseDrag(const juce::MouseEvent& e)
{
    TextButton::mouseDrag(e); if (started || e.getDistanceFromDragStart() < 5 || !isShowing()) return;
    started = true;
    const auto directory = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("RaBubanj-MIDI");
    if (directory.createDirectory().failed()) { setButtonText("MIDI ERROR"); return; }
    const auto file = directory.getChildFile("RaBubanj-"+juce::Uuid().toString()+".mid");
    if (!processor.writeSequenceMidi(file)) { setButtonText("MIDI ERROR"); return; }
    const juce::Component::SafePointer<MidiPatternDrag> safe(this);
    // Hosts may read after mouse-up. Keep exports in the OS temp directory, not a short-lived TemporaryFile.
    if (!juce::DragAndDropContainer::performExternalDragDropOfFiles({ file.getFullPathName() }, false, this, [safe] { if (safe) safe->setButtonText("DRAG MIDI"); })) setButtonText("MIDI ERROR");
}
SequencerPanel::SequencerPanel(RaBubanjProcessor& p) : processor(p), grid(p), midiDrag(p)
{
    setName("Step sequencer"); setWantsKeyboardFocus(false); addAndMakeVisible(grid); addAndMakeVisible(midiDrag);
    enabled.setName("Sequencer enabled"); enabled.setClickingTogglesState(true); enabled.setWantsKeyboardFocus(false); addAndMakeVisible(enabled);
    enabled.setTooltip("Follow the DAW transport. Disable when playing an exported MIDI clip.");
    enabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters, "seqEnabled", enabled);
    const std::array<const char*, 3> labels { "SLABA", "SREDNJA", "JAKA" };
    for (int i = 0; i < 3; ++i)
    {
        auto& button = velocity[static_cast<size_t>(i)]; button.setButtonText(labels[static_cast<size_t>(i)]); button.setName("Velocity "+juce::String(i+1));
        button.setWantsKeyboardFocus(false); button.setColour(juce::TextButton::buttonOnColourId, gold);
        button.onClick = [this, i] { processor.setSequenceBrush(i+1); refresh(); }; addAndMakeVisible(button);
    }
    setSize(1040, 440); refresh(); startTimerHz(30);
}
SequencerPanel::~SequencerPanel() { stopTimer(); }
void SequencerPanel::refresh()
{
    enabled.setButtonText(enabled.getToggleState() ? "SEQ ON" : "SEQ OFF");
    for (int i = 0; i < 3; ++i) velocity[static_cast<size_t>(i)].setToggleState(processor.getSequenceBrush() == i+1, juce::dontSendNotification);
    grid.repaint(); repaint();
}
void SequencerPanel::resized()
{
    auto header = getLocalBounds().reduced(16, 12).removeFromTop(29);
    enabled.setBounds(header.removeFromLeft(90)); header.removeFromLeft(18);
    for (auto& button : velocity) { button.setBounds(header.removeFromLeft(95).reduced(3, 0)); }
    midiDrag.setBounds(header.removeFromRight(122));
    grid.setBounds(getLocalBounds().reduced(12).withTrimmedTop(64).withTrimmedBottom(30));
}
void SequencerPanel::paint(juce::Graphics& g)
{
    raisedPanel(g, getLocalBounds().toFloat(), panel);
    const auto status = !processor.hasSequenceHostPosition() ? "WAITING FOR DAW" : processor.isSequenceHostPlaying() ? "DAW PLAY" : "DAW STOP";
    text(g, juce::String(status)+"  /  "+juce::String(processor.getHostBpm(), 2)+" BPM  /  1/16  /  2 BARS", { 20, 43, getWidth()-40, 24 }, 12, ink, juce::Justification::centredLeft, true);
    text(g, "RIGHT: WRITE    LEFT: ERASE    HOLD + DRAG LEFT: ERASE MULTIPLE    SPACE: DAW TRANSPORT", { 20, getHeight()-34, getWidth()-40, 24 }, 11, ink, juce::Justification::centredLeft, true);
}
SequencerWindow::SequencerWindow(RaBubanjProcessor& p)
    : DocumentWindow("RaBubanj / STEP SEQUENCER", panel, closeButton, false)
{
    setUsingNativeTitleBar(false); setTitleBarHeight(30); setWantsKeyboardFocus(false);
    setContentOwned(new SequencerPanel(p), true); setResizable(false, false);
    if (auto* close = getCloseButton()) close->setWantsKeyboardFocus(false);
}
