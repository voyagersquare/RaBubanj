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
    woodText(g, juce::String(rabubanj::padNames[static_cast<size_t>(row)])+" / STEP "+juce::String(step+1), { 12, 7, getWidth()-50, 22 }, 16);
    woodText(g, "EARLIER               TIMING               LATER", { 16, 34, getWidth()-32, 20 }, 12, juce::Justification::centred);
    woodText(g, "DOUBLE-CLICK SLIDER: RESET TO GRID", { 10, 111, getWidth()-20, 18 }, 12, juce::Justification::centred);
}
StepGrid::StepGrid(RaBubanjProcessor& p) : processor(p)
{
    setName("Step grid"); setWantsKeyboardFocus(false);
    horizontalScroll.setName("Sequence horizontal scroll"); horizontalScroll.setWantsKeyboardFocus(false);
    horizontalScroll.setSingleStepSize(4); horizontalScroll.addListener(this); addChildComponent(horizontalScroll);
    horizontalScroll.setColour(juce::ScrollBar::backgroundColourId, dark);
    horizontalScroll.setColour(juce::ScrollBar::thumbColourId, axis); refresh();
}
void StepGrid::resized() { updateScrollBar(); }
void StepGrid::updateScrollBar()
{
    const auto length = processor.getPattern().getLength(); firstVisibleStep = juce::jlimit(0, length-32, firstVisibleStep);
    horizontalScroll.setRangeLimits(0, length, juce::dontSendNotification);
    horizontalScroll.setCurrentRange(firstVisibleStep, 32, juce::dontSendNotification);
    const auto r = plot(); horizontalScroll.setBounds(juce::roundToInt(r.getX()), getHeight()-19, juce::roundToInt(r.getWidth()), 14);
    horizontalScroll.setVisible(length > 32);
}
void StepGrid::scrollToStep(int step)
{
    const auto next = juce::jlimit(0, processor.getPattern().getLength()-32, step);
    if (firstVisibleStep == next) return;
    firstVisibleStep = next; timingEditor.reset(); updateScrollBar(); repaint();
}
void StepGrid::mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
{
    if (drawing || leftPending) return;
    const auto delta = std::abs(wheel.deltaX) > std::abs(wheel.deltaY) ? wheel.deltaX : wheel.deltaY;
    if (delta == 0) return;
    const auto distance = juce::jmax(1, juce::roundToInt(std::abs(delta)*32));
    scrollToStep(firstVisibleStep+(delta < 0 ? distance : -distance));
}
juce::Rectangle<float> StepGrid::plot() const { return getLocalBounds().toFloat().reduced(8, 4).withTrimmedLeft(86).withTrimmedTop(50).withTrimmedBottom(processor.getPattern().getLength() > 32 ? 20.0f : 0.0f); }
juce::Rectangle<float> StepGrid::cellBounds(int row, int step) const
{
    const auto r = plot(); return { r.getX()+static_cast<float>(step-firstVisibleStep)*r.getWidth()/32, r.getY()+static_cast<float>(row)*r.getHeight()/8, r.getWidth()/32, r.getHeight()/8 };
}
juce::Rectangle<float> StepGrid::padBounds(int row) const
{ const auto r = plot(); return { r.getX()-82, r.getY()+static_cast<float>(row)*r.getHeight()/8, 74, r.getHeight()/8 }; }
juce::Point<int> StepGrid::cellAt(juce::Point<float> p, bool clamp) const
{
    const auto r = plot(); if (!clamp && !r.contains(p)) return { -1, -1 };
    if (r.getWidth() <= 0 || r.getHeight() <= 0) return { -1, -1 };
    return { juce::jlimit(firstVisibleStep, firstVisibleStep+31, firstVisibleStep+static_cast<int>(std::floor((p.x-r.getX())*32/r.getWidth()))), juce::jlimit(0, 7, static_cast<int>(std::floor((p.y-r.getY())*8/r.getHeight()))) };
}
void StepGrid::stroke(juce::Point<int> cell)
{
    if (cell.x < 0 || cell.y < 0) return;
    cell.x = juce::jmin(cell.x, processor.getPattern(editPattern).getLength()-1);
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
    const auto selected = processor.getSelectedPattern(), length = processor.getPattern().getLength();
    if (selected != displayedPattern || length != displayedLength)
    {
        if (selected == displayedPattern && length != displayedLength)
        {
            stopTimer(); if (changed) processor.sequenceEdited();
            drawing = changed = leftPending = longOpened = false; last = { -1, -1 };
        }
        if (selected != displayedPattern) firstVisibleStep = 0;
        displayedPattern = selected; displayedLength = length; timingEditor.reset(); updateScrollBar();
    }
    if (timingEditor && timingEditor->patternIndex != processor.getSelectedPattern()) timingEditor.reset();
    if (timingEditor) timingEditor->refresh();
    repaint();
}
void StepGrid::paint(juce::Graphics& g)
{
    insetPanel(g, getLocalBounds().toFloat(), true); const auto r = plot(); const auto playing = processor.getPlayingStep();
    const auto cells = processor.getPattern().timedSnapshot();
    {
    juce::Graphics::ScopedSaveState saved(g);
    g.reduceClipRegion(juce::Rectangle<float>(r.getX(), 5, r.getWidth(), r.getBottom()-5).toNearestInt());
    for (int group = firstVisibleStep/8; group <= (firstVisibleStep+31)/8; ++group)
    {
        const auto first = cellBounds(0, group*8), lastBox = cellBounds(0, group*8+7);
        const auto heading = juce::Rectangle<float>(first.getX(), 8, lastBox.getRight()-first.getX(), 19);
        text(g, "BAR "+juce::String(group/2+1)+"  /  "+juce::String(group*8+1)+"–"+juce::String(group*8+8), heading.toNearestInt(), 13, axis, juce::Justification::centred, true);
        if (group > 0)
        {
            g.setColour(group%2 == 0 ? gold.withAlpha(.85f) : axis.withAlpha(.55f));
            g.fillRect(first.getX()-(group%2 == 0 ? 1.5f : 1), 8.0f, group%2 == 0 ? 3.0f : 2.0f, r.getBottom()-8);
        }
    }
    for (int step = firstVisibleStep; step < firstVisibleStep+32; ++step)
    {
        const auto box = cellBounds(0, step);
        if (step == playing) { g.setColour(gold.withAlpha(.18f)); g.fillRect(box.withHeight(r.getHeight()).expanded(0, 1)); }
        text(g, juce::String(step+1), { juce::roundToInt(box.getX()), 28, juce::roundToInt(box.getWidth()), 22 }, 13, step == playing ? gold : ink, juce::Justification::centred, true);
        for (int row = 0; row < 8; ++row)
        {
            auto cell = cellBounds(row, step).reduced(2, 3);
            const auto point = cells[static_cast<size_t>(row*rabubanj::StepPattern::steps+step)]; const auto value = point.strength;
            g.setColour(step/4%2 == 0 ? juce::Colour(0xff302e24) : juce::Colour(0xff62563a));
            g.fillRoundedRectangle(cell, 2);
            if (value > 0)
            {
                const std::array<juce::Colour, 4> colours { juce::Colour(0xff786c49), juce::Colour(0xffa28b4a), juce::Colour(0xffcdb057), juce::Colour(0xffffd46b) };
                g.setColour(colours[static_cast<size_t>(value-1)]);
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
    }
    for (int row = 0; row < 8; ++row)
    {
        const auto label = padBounds(row).reduced(0, 3);
        flatCaption(g, label, row == selectedPad ? juce::Colour(0xff344739) : juce::Colour(0xff142019).withAlpha(.86f),
            row == selectedPad ? gold.withAlpha(.8f) : juce::Colour(0xffe9d6a5).withAlpha(.45f));
        text(g, rabubanj::padNames[static_cast<size_t>(row)], label.toNearestInt(), 16, ink, juce::Justification::centred, true);
    }
}
MidiPatternDrag::MidiPatternDrag(RaBubanjProcessor& p) : TextButton("DRAG MIDI"), processor(p)
{
    setName("Drag sequence MIDI"); setWantsKeyboardFocus(false);
    setTooltip("Drag the selected MIDI pattern (32, 64 or 128 steps) into the DAW. Switch SEQ OFF when playing the exported MIDI clip to avoid doubled hits.");
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
    lengthChoice.setName("Pattern length"); lengthChoice.setWantsKeyboardFocus(false);
    lengthChoice.setTooltip("Length of this pattern. Scroll the grid with the mouse wheel or the horizontal bar to reach later steps.");
    for (int length : { 32, 64, 128 }) lengthChoice.addItem(juce::String(length)+" STEPS", length);
    lengthChoice.onChange = [this]
    {
        if (processor.getPattern().setLength(lengthChoice.getSelectedId())) processor.sequenceEdited();
        grid.refresh(); repaint();
    };
    addAndMakeVisible(lengthChoice);
    patternAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.parameters, "seqPattern", patternChoice);
    setSize(1160, 620); refresh(); startTimerHz(30);
}
SequencerPanel::~SequencerPanel() { stopTimer(); processor.getWaveformTap().selectPad(-1); }
void SequencerPanel::visibilityChanged() { waveform.update(grid.getSelectedPad(), isShowing()); }
void SequencerPanel::refresh()
{
    enabled.setButtonText(enabled.getToggleState() ? "SEQ ON" : "SEQ OFF");

    lengthChoice.setSelectedId(processor.getPattern().getLength(), juce::dontSendNotification);
    waveform.update(grid.getSelectedPad(), isShowing()); grid.refresh(); repaint();
}
void SequencerPanel::resized()
{
    auto header = getLocalBounds().reduced(16, 12).removeFromTop(29);
    enabled.setBounds(header.removeFromLeft(90)); header.removeFromLeft(18);
    patternChoice.setBounds(header.removeFromLeft(220)); header.removeFromLeft(18);
    lengthChoice.setBounds(header.removeFromLeft(145));
    midiDrag.setBounds(header.removeFromRight(122));
    grid.setBounds(getLocalBounds().reduced(12).withTrimmedTop(68).withTrimmedBottom(170));
    waveform.setBounds(12, getHeight()-168, getWidth()-24, 128);
}
void SequencerPanel::paint(juce::Graphics& g)
{
    raisedPanel(g, getLocalBounds().toFloat(), panel);
    const auto status = !processor.hasSequenceHostPosition() ? "WAITING FOR DAW" : processor.isSequenceHostPlaying() ? "DAW PLAY" : "DAW STOP";
    const auto length = processor.getPattern().getLength(), first = grid.getFirstVisibleStep();
    woodText(g, juce::String(status)+"  /  "+juce::String(processor.getHostBpm(), 2)+" BPM  /  1/16  /  "+juce::String(length/16)+" BARS  /  VIEW "+juce::String(first+1)+"–"+juce::String(first+32)+" / "+juce::String(length), { 20, 43, getWidth()-40, 26 }, 14);
    woodText(g, "LEFT: CLICK / CYCLE VELOCITY   DRAG: WRITE   HOLD: NOTE TIMING   RIGHT / DRAG: ERASE   WHEEL: SCROLL", { 20, getHeight()-34, getWidth()-40, 24 }, 14);
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
    insetPanel(g, getLocalBounds().toFloat(), true);
    const auto info = processor.getSampleInfo(selectedPad);
    text(g, juce::String(rabubanj::padNames[static_cast<size_t>(selectedPad)])+" / LIVE WAVEFORM"+(info.name.isNotEmpty() ? " / "+info.name : ""), { 12, 5, getWidth()-24, 23 }, 14, ink, juce::Justification::centredLeft, true);
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
    setSize(1160, 650);
}
void SequencerWindow::resized() { close.setBounds(getWidth()-36, 3, 31, 25); content.setBounds(0, 30, getWidth(), getHeight()-30); }
void SequencerWindow::paint(juce::Graphics& g)
{ raisedPanel(g, getLocalBounds().toFloat(), panel); woodText(g, getName(), { 40, 0, getWidth()-80, 30 }, 17, juce::Justification::centred); }
