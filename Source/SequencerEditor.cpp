#include "SequencerEditor.h"
using namespace rabubanj::ui;
StepGrid::StepGrid(RaBubanjProcessor& p) : processor(p) { setName("32 step grid"); setWantsKeyboardFocus(false); }
juce::Rectangle<float> StepGrid::plot() const { return getLocalBounds().toFloat().reduced(8, 4).withTrimmedLeft(86).withTrimmedTop(27); }
juce::Rectangle<float> StepGrid::cellBounds(int row, int step) const
{
    const auto r = plot(); return { r.getX()+static_cast<float>(step)*r.getWidth()/32, r.getY()+static_cast<float>(row)*r.getHeight()/8, r.getWidth()/32, r.getHeight()/8 };
}
juce::Rectangle<float> StepGrid::padBounds(int row) const
{ const auto box = cellBounds(row, 0); return { plot().getX()-82, box.getY(), 74, box.getHeight() }; }
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
        changed = processor.getPattern(editPattern).set(previous.y, previous.x, 0) || changed;
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
    if (e.mods.isLeftButtonDown()) for (int row = 0; row < 8; ++row) if (padBounds(row).contains(e.position))
    { selectedPad = row; processor.selectChannel(row); processor.audition(row); repaint(); return; }
    last = { -1, -1 }; changed = false; editPattern = processor.getSelectedPattern();
    const auto cell = cellAt(e.position, false);
    drawing = cell.x >= 0 && e.mods.isRightButtonDown();
    if (drawing) stroke(cell);
    else if (cell.x >= 0 && e.mods.isLeftButtonDown())
    { processor.getPattern(editPattern).cycle(cell.y, cell.x); changed = true; repaint(); }
}
void StepGrid::mouseDrag(const juce::MouseEvent& e) { if (drawing) stroke(cellAt(e.position, true)); }
void StepGrid::mouseUp(const juce::MouseEvent&)
{ if (changed) processor.sequenceEdited(); drawing = changed = false; last = { -1, -1 }; }
void StepGrid::paint(juce::Graphics& g)
{
    insetPanel(g, getLocalBounds().toFloat()); const auto r = plot(); const auto playing = processor.getPlayingStep();
    const auto cells = processor.getPattern().snapshot();
    for (int step = 0; step < 32; ++step)
    {
        const auto box = cellBounds(0, step);
        if (step == playing) { g.setColour(gold.withAlpha(.18f)); g.fillRect(box.withHeight(r.getHeight()).expanded(0, 1)); }
        text(g, juce::String(step+1), { juce::roundToInt(box.getX()), 5, juce::roundToInt(box.getWidth()), 22 }, 11, step == playing ? gold : ink, juce::Justification::centred, true);
        for (int row = 0; row < 8; ++row)
        {
            auto cell = cellBounds(row, step).reduced(2, 3);
            const auto value = cells[static_cast<size_t>(row*32+step)];
            g.setColour(step/4%2 == 0 ? juce::Colour(0xff62563a) : juce::Colour(0xff302e24)); g.fillRoundedRectangle(cell, 2);
            if (value > 0)
            {
                const std::array<juce::Colour, 4> colours { juce::Colour(0xff786c49), juce::Colour(0xffa28b4a), juce::Colour(0xffcdb057), juce::Colour(0xffffd46b) };
                g.setColour(colours[static_cast<size_t>(value-1)]);
                auto fill = cell.reduced(2); fill.removeFromTop(fill.getHeight()*(1-static_cast<float>(value)/4)); g.fillRoundedRectangle(fill, 1);
            }
            g.setColour(step == playing ? gold : juce::Colour(0xff6e5b2b)); g.drawRoundedRectangle(cell, 2, step == playing ? 1.4f : .7f);
        }
    }
    for (int row = 0; row < 8; ++row)
    {
        const auto label = padBounds(row).reduced(0, 3);
        if (row == selectedPad) { g.setColour(juce::Colour(0xff89733d)); g.fillRoundedRectangle(label, 3); g.setColour(gold); g.drawRoundedRectangle(label, 3, 1); }
        text(g, rabubanj::padNames[static_cast<size_t>(row)], label.toNearestInt(), 14, ink, juce::Justification::centred, true);
    }
}
MidiPatternDrag::MidiPatternDrag(RaBubanjProcessor& p) : TextButton("DRAG MIDI"), processor(p)
{
    setName("Drag sequence MIDI"); setWantsKeyboardFocus(false);
    setTooltip("Drag the selected two-bar MIDI pattern into the DAW. Switch SEQ OFF when playing the exported MIDI clip to avoid doubled hits.");
}
void MidiPatternDrag::mouseDown(const juce::MouseEvent& e) { started = false; TextButton::mouseDown(e); }
void MidiPatternDrag::mouseDrag(const juce::MouseEvent& e)
{
    TextButton::mouseDrag(e); if (started || e.getDistanceFromDragStart() < 5 || !isShowing()) return;
    started = true;
    const auto directory = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("RaBubanj-MIDI");
    if (directory.createDirectory().failed()) { setButtonText("MIDI ERROR"); return; }
    const auto selected = processor.getSelectedPattern();
    const auto file = directory.getChildFile("RaBubanj-Pattern-"+juce::String(selected+1)+"-"+juce::Uuid().toString()+".mid");
    if (!processor.writeSequenceMidi(file, selected)) { setButtonText("MIDI ERROR"); return; }
    const juce::Component::SafePointer<MidiPatternDrag> safe(this);
    // Hosts may read after mouse-up. Keep exports in the OS temp directory, not a short-lived TemporaryFile.
    if (!juce::DragAndDropContainer::performExternalDragDropOfFiles({ file.getFullPathName() }, false, this, [safe] { if (safe) safe->setButtonText("DRAG MIDI"); })) setButtonText("MIDI ERROR");
}
SequencerPanel::SequencerPanel(RaBubanjProcessor& p) : processor(p), grid(p), waveform(p), midiDrag(p)
{
    setName("Step sequencer"); setWantsKeyboardFocus(false); addAndMakeVisible(grid); addAndMakeVisible(midiDrag); addAndMakeVisible(waveform);
    enabled.setName("Sequencer enabled"); enabled.setClickingTogglesState(true); enabled.setWantsKeyboardFocus(false); addAndMakeVisible(enabled);
    enabled.setTooltip("Follow the DAW transport. Disable when playing an exported MIDI clip.");
    enabledAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters, "seqEnabled", enabled);
    patternChoice.setName("Pattern selector"); patternChoice.setWantsKeyboardFocus(false);
    patternChoice.setTooltip("Select one of four independent patterns. Playback keeps its DAW phase; MIDI export uses this pattern.");
    for (int i = 0; i < 4; ++i) patternChoice.addItem("Pattern "+juce::String(i+1), i+1);
    addAndMakeVisible(patternChoice);
    patternAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.parameters, "seqPattern", patternChoice);
    setSize(1160, 620); refresh(); startTimerHz(30);
}
SequencerPanel::~SequencerPanel() { stopTimer(); processor.getWaveformTap().selectPad(-1); }
void SequencerPanel::visibilityChanged() { waveform.update(grid.getSelectedPad(), isShowing()); }
void SequencerPanel::refresh()
{
    enabled.setButtonText(enabled.getToggleState() ? "SEQ ON" : "SEQ OFF");

    waveform.update(grid.getSelectedPad(), isShowing()); grid.repaint(); repaint();
}
void SequencerPanel::resized()
{
    auto header = getLocalBounds().reduced(16, 12).removeFromTop(29);
    enabled.setBounds(header.removeFromLeft(90)); header.removeFromLeft(18);
    patternChoice.setBounds(header.removeFromLeft(220));
    midiDrag.setBounds(header.removeFromRight(122));
    grid.setBounds(getLocalBounds().reduced(12).withTrimmedTop(64).withTrimmedBottom(170));
    waveform.setBounds(12, getHeight()-168, getWidth()-24, 128);
}
void SequencerPanel::paint(juce::Graphics& g)
{
    raisedPanel(g, getLocalBounds().toFloat(), panel);
    const auto status = !processor.hasSequenceHostPosition() ? "WAITING FOR DAW" : processor.isSequenceHostPlaying() ? "DAW PLAY" : "DAW STOP";
    text(g, juce::String(status)+"  /  "+juce::String(processor.getHostBpm(), 2)+" BPM  /  1/16  /  2 BARS", { 20, 43, getWidth()-40, 24 }, 12, ink, juce::Justification::centredLeft, true);
    text(g, "LEFT: WRITE / CYCLE 4 VELOCITIES    RIGHT: ERASE    HOLD + DRAG RIGHT: ERASE MULTIPLE", { 20, getHeight()-34, getWidth()-40, 24 }, 11, ink, juce::Justification::centredLeft, true);
}
void PadWaveform::update(int pad, bool active)
{
    selectedPad = pad; auto& tap = processor.getWaveformTap(); tap.selectPad(active ? pad : -1);
    const auto next = tap.getToken(); if (next != token) { history = {}; position = 0; token = next; }
    std::array<rabubanj::WaveformTap::Frame, 512> frames; int count;
    while ((count = tap.read(frames.data(), static_cast<int>(frames.size()))) > 0)
        for (int i = 0; i < count; ++i) if (frames[static_cast<size_t>(i)].token == token)
        { history[static_cast<size_t>(position)] = frames[static_cast<size_t>(i)]; position = (position+1)%static_cast<int>(history.size()); }
    repaint();
}
float PadWaveform::getDisplayedPeak() const noexcept
{ float peak = 0; for (const auto& frame : history) peak = std::max(peak, std::max(std::abs(frame.low), std::abs(frame.high))); return peak; }
void PadWaveform::paint(juce::Graphics& g)
{
    insetPanel(g, getLocalBounds().toFloat());
    const auto info = processor.getSampleInfo(selectedPad);
    text(g, juce::String(rabubanj::padNames[static_cast<size_t>(selectedPad)])+" / LIVE WAVEFORM"+(info.name.isNotEmpty() ? " / "+info.name : ""), { 12, 5, getWidth()-24, 23 }, 12, ink, juce::Justification::centredLeft, true);
    const auto plot = getLocalBounds().toFloat().reduced(12, 7).withTrimmedTop(25); const auto cy = plot.getCentreY();
    g.setColour(grid); g.drawHorizontalLine(juce::roundToInt(cy), plot.getX(), plot.getRight());
    g.setColour(gold); const auto columns = juce::jmax(1, juce::roundToInt(plot.getWidth()));
    for (int x = 0; x < columns; ++x)
    {
        const auto begin = x*static_cast<int>(history.size())/columns, end = juce::jmax(begin+1, (x+1)*static_cast<int>(history.size())/columns);
        float low = 0, high = 0;
        for (int i = begin; i < end; ++i) { const auto& frame = history[static_cast<size_t>((position+i)%static_cast<int>(history.size()))]; low = std::min(low, frame.low); high = std::max(high, frame.high); }
        g.drawVerticalLine(juce::roundToInt(plot.getX())+x, cy-juce::jlimit(0.0f, 1.0f, high)*plot.getHeight()*.46f, cy-juce::jlimit(-1.0f, 0.0f, low)*plot.getHeight()*.46f+.6f);
    }
}
SequencerWindow::SequencerWindow(RaBubanjProcessor& p) : content(p)
{
    setName("RaBubanj / STEP SEQUENCER"); setWantsKeyboardFocus(false);
    close.setName("Close sequencer"); close.setWantsKeyboardFocus(false); close.setTooltip("Close step sequencer");
    close.onClick = [this] { closeButtonPressed(); }; addAndMakeVisible(content); addAndMakeVisible(close);
    constrainer.setMinimumOnscreenAmounts(650, 200, 40, 200); setSize(1160, 650);
}
void SequencerWindow::resized() { close.setBounds(getWidth()-36, 3, 31, 25); content.setBounds(0, 30, getWidth(), getHeight()-30); }
void SequencerWindow::paint(juce::Graphics& g)
{ g.fillAll(menuFace); g.setColour(gold); g.drawRect(getLocalBounds(), 1); text(g, getName(), { 40, 0, getWidth()-80, 30 }, 15, ink, juce::Justification::centred, true); }
void SequencerWindow::mouseDown(const juce::MouseEvent& e) { if (e.position.y < 30) dragger.startDraggingComponent(this, e); }
void SequencerWindow::mouseDrag(const juce::MouseEvent& e) { dragger.dragComponent(this, e, &constrainer); }
