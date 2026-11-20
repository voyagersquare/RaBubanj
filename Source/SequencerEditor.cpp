#include "SequencerEditor.h"
using namespace rabubanj::ui;
NoteTimingEditor::NoteTimingEditor(RaBubanjProcessor& p, int pattern, int r, int s)
    : patternIndex(pattern), processor(p), row(r), step(s)
{
    setName("Note timing editor"); setWantsKeyboardFocus(false);
    rabubanj::ui::slider(timing, juce::Slider::LinearHorizontal, 210); timing.setName("Note timing");
    timing.setRange(-50, 50, 1); timing.setWantsKeyboardFocus(false); timing.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 210, 22);
    timing.setDoubleClickReturnValue(true, 0); timing.setValue(p.getPattern(patternIndex).getTiming(row, step), juce::dontSendNotification);
    timing.textFromValueFunction = [this](double v) { return (v > 0 ? "+" : "")+juce::String(v, 0)+"%  /  "+juce::String(v*150/processor.getHostBpm(), 1)+" ms"; };
    timing.onValueChange = [this] { if (processor.getPattern(patternIndex).setTiming(row, step, juce::roundToInt(timing.getValue()))) processor.sequenceEdited(); if (auto* parent = getParentComponent()) parent->repaint(); };
    timing.updateText(); addAndMakeVisible(timing);
    close.setName("Close note timing"); close.setWantsKeyboardFocus(false); close.onClick = [this] { setVisible(false); }; addAndMakeVisible(close); setSize(310, 136);
}
void NoteTimingEditor::refresh() { timing.updateText(); }
void NoteTimingEditor::resized() { close.setBounds(getWidth()-31, 5, 25, 22); timing.setBounds(16, 52, getWidth()-32, 55); }
void NoteTimingEditor::paint(juce::Graphics& g)
{
    raisedPanel(g, getLocalBounds().toFloat(), menuFace);
    text(g, juce::String(rabubanj::padNames[static_cast<size_t>(row)])+" / STEP "+juce::String(step+1), { 12, 7, getWidth()-50, 22 }, 14, ink, juce::Justification::centredLeft, true);
    text(g, "EARLIER                    TIMING                    LATER", { 16, 34, getWidth()-32, 20 }, 11, axis, juce::Justification::centred, true);
    text(g, "DOUBLE-CLICK SLIDER: RESET TO GRID", { 10, 111, getWidth()-20, 18 }, 10, axis, juce::Justification::centred, true);
}
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
        auto& pattern = processor.getPattern(editPattern);
        if (erasing) changed = pattern.set(previous.y, previous.x, 0) || changed;
        else if (pattern.get(previous.y, previous.x) == 0) changed = pattern.set(previous.y, previous.x, 4) || changed;
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
    stopTimer(); timingEditor.reset(); leftPending = longOpened = drawing = changed = false; last = { -1, -1 };
    if (e.mods.isLeftButtonDown()) for (int row = 0; row < 8; ++row) if (padBounds(row).contains(e.position))
    { selectedPad = row; processor.selectChannel(row); processor.audition(row); repaint(); return; }
    editPattern = processor.getSelectedPattern(); pressedCell = cellAt(e.position, false);
    if (pressedCell.x < 0) return;
    erasing = e.mods.isRightButtonDown(); drawing = erasing;
    if (erasing) stroke(pressedCell);
    else { leftPending = true; downPosition = e.position; downTime = e.eventTime; startTimer(650); }
}
void StepGrid::mouseDrag(const juce::MouseEvent& e)
{
    if (longOpened) return;
    if (leftPending && e.position.getDistanceFrom(downPosition) >= 4)
    { stopTimer(); leftPending = false; drawing = true; stroke(pressedCell); }
    if (drawing) stroke(cellAt(e.position, true));
}
void StepGrid::timerCallback() { stopTimer(); if (leftPending) openTimingEditor(); }
void StepGrid::openTimingEditor()
{
    if (processor.getSelectedPattern() != editPattern) { leftPending = false; return; }
    auto& pattern = processor.getPattern(editPattern);
    if (pattern.get(pressedCell.y, pressedCell.x) == 0) changed = pattern.set(pressedCell.y, pressedCell.x, 4) || changed;
    timingEditor = std::make_unique<NoteTimingEditor>(processor, editPattern, pressedCell.y, pressedCell.x);
    const auto anchor = cellBounds(pressedCell.y, pressedCell.x);
    timingEditor->setTopLeftPosition(juce::jlimit(4, juce::jmax(4, getWidth()-314), juce::roundToInt(anchor.getCentreX())-155),
        juce::jlimit(4, juce::jmax(4, getHeight()-140), juce::roundToInt(anchor.getBottom())));
    addAndMakeVisible(*timingEditor); timingEditor->toFront(false); longOpened = true; leftPending = false; repaint();
}
void StepGrid::mouseUp(const juce::MouseEvent& e)
{
    stopTimer();
    if (leftPending && (e.eventTime-downTime).inMilliseconds() >= 650) openTimingEditor();
    if (leftPending) { processor.getPattern(editPattern).cycle(pressedCell.y, pressedCell.x); changed = true; }
    if (changed) processor.sequenceEdited();
    drawing = changed = leftPending = false; last = { -1, -1 }; repaint();
}
void StepGrid::refresh()
{
    if (timingEditor && timingEditor->patternIndex != processor.getSelectedPattern()) timingEditor.reset();
    if (timingEditor) timingEditor->refresh();
    repaint();
}
void StepGrid::paint(juce::Graphics& g)
{
    insetPanel(g, getLocalBounds().toFloat()); const auto r = plot(); const auto playing = processor.getPlayingStep();
    const auto cells = processor.getPattern().timedSnapshot();
    for (int step = 0; step < 32; ++step)
    {
        const auto box = cellBounds(0, step);
        if (step == playing) { g.setColour(gold.withAlpha(.18f)); g.fillRect(box.withHeight(r.getHeight()).expanded(0, 1)); }
        text(g, juce::String(step+1), { juce::roundToInt(box.getX()), 5, juce::roundToInt(box.getWidth()), 22 }, 11, step == playing ? gold : ink, juce::Justification::centred, true);
        for (int row = 0; row < 8; ++row)
        {
            auto cell = cellBounds(row, step).reduced(2, 3);
            const auto point = cells[static_cast<size_t>(row*32+step)]; const auto value = point.strength;
            raisedMetal(g, cell, step/4%2 == 0 ? steel.darker(.22f) : steelDark, 4);
            g.setColour(juce::Colours::black.withAlpha(step/4%2 == 0 ? .08f : .26f)); g.fillRoundedRectangle(cell.reduced(2).withTrimmedBottom(4), 1);
            if (value > 0)
            {
                const std::array<juce::Colour, 4> colours { juce::Colour(0xff786c49), juce::Colour(0xffa28b4a), juce::Colour(0xffcdb057), juce::Colour(0xffffd46b) };
                g.setColour(colours[static_cast<size_t>(value-1)].withAlpha(.8f));
                auto fill = cell.reduced(2); fill.removeFromTop(fill.getHeight()*(1-static_cast<float>(value)/4)); g.fillRoundedRectangle(fill, 1);
            }
            if (value > 0 && point.timing != 0)
            {
                const auto x = cell.getCentreX()+static_cast<float>(point.timing)*cell.getWidth()*.007f;
                g.setColour(point.timing < 0 ? cyan : juce::Colour(0xffed9b59)); g.fillEllipse(x-2, cell.getY()+2, 4, 4);
            }
            g.setColour(step == playing ? gold : juce::Colour(0xff6e5b2b)); g.drawRoundedRectangle(cell, 2, step == playing ? 1.4f : .7f);
        }
    }
    for (int row = 0; row < 8; ++row)
    {
        const auto label = padBounds(row).reduced(0, 3);
        raisedMetal(g, label, row == selectedPad ? steel.brighter(.3f) : steel.darker(.3f), 4);
        if (row == selectedPad) { g.setColour(gold.withAlpha(.25f)); g.fillRoundedRectangle(label.reduced(2), 2); }
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

    waveform.update(grid.getSelectedPad(), isShowing()); grid.refresh(); repaint();
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
    text(g, "LEFT: CLICK / CYCLE VELOCITY   DRAG: WRITE   HOLD: NOTE TIMING   RIGHT / DRAG: ERASE", { 20, getHeight()-34, getWidth()-40, 24 }, 11, ink, juce::Justification::centredLeft, true);
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
{ raisedPanel(g, getLocalBounds().toFloat(), panel); text(g, getName(), { 40, 0, getWidth()-80, 30 }, 15, ink, juce::Justification::centred, true); }
void SequencerWindow::mouseDown(const juce::MouseEvent& e) { if (e.position.y < 30) dragger.startDraggingComponent(this, e); }
void SequencerWindow::mouseDrag(const juce::MouseEvent& e) { dragger.dragComponent(this, e, &constrainer); }
