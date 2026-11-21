#include "PluginEditor.h"
#include <cstdlib>
#include <iostream>
static void require(bool ok, const char* text) { if (!ok) { std::cerr << "FAIL: " << text << '\n'; std::exit(1); } }
static void set(RaBubanjProcessor& p, const juce::String& id, float value)
{ auto* parameter = p.parameters.getParameter(id); parameter->setValueNotifyingHost(parameter->convertTo0to1(value)); }
struct Head : juce::AudioPlayHead
{
    double ppq = 0, bpm = 120; bool playing = true;
    juce::Optional<PositionInfo> getPosition() const override
    { PositionInfo info; info.setPpqPosition(ppq); info.setBpm(bpm); info.setIsPlaying(playing); return info; }
};
int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    Head head; RaBubanjProcessor p; p.setPlayHead(&head); p.setRateAndBufferSizeDetails(48000, 512); p.prepareToPlay(48000, 512);
    p.getPattern().set(0, 0, 4); head.ppq = -.01;
    juce::AudioBuffer<float> audio(2, 512); juce::MidiBuffer midi; p.processBlock(audio, midi);
    require(audio.getMagnitude(0, 240) == 0 && audio.getMagnitude(241, 271) > .01f && p.getHitCount(0) == 1, "host-synced step must start at its exact audio offset");
    head.playing = false; const auto beforeStop = p.getHitCount(0); p.processBlock(audio, midi);
    require(p.getHitCount(0) == beforeStop && p.getPlayingStep() == -1, "DAW stop must stop generating sequence notes");
    head.playing = true; head.ppq = 0; set(p, "seqEnabled", 0); p.processBlock(audio, midi);
    require(p.getHitCount(0) == beforeStop, "SEQ OFF must disable internal notes while the DAW plays");
    set(p, "seqEnabled", 1); set(p, "midiMap", 1); p.getPattern().set(1, 0, 2);
    const auto beforePad2 = p.getHitCount(1); p.processBlock(audio, midi);
    require(p.getHitCount(1) == beforePad2+1, "sequence rows must trigger pad indices independently of MIDI mapping mode");
    head.ppq = -.01; p.releaseResources(); p.prepareToPlay(48000, 512);
    midi.addEvent(juce::MidiMessage::allSoundOff(1), 240); p.processBlock(audio, midi);
    require(audio.getMagnitude(0, 512) == 0, "panic at a step boundary must silence the scheduled hit at that same offset");
    for (int index = 0; index < 4; ++index) { p.getPattern(index).replace({}); p.getPattern(index).set(index, 0, 4-index); }
    head.ppq = 0;
    for (int index = 0; index < 4; ++index)
    {
        std::array<uint32_t, 8> before {}; for (int row = 0; row < 8; ++row) before[static_cast<size_t>(row)] = p.getHitCount(row);
        set(p, "seqPattern", static_cast<float>(index)); p.processBlock(audio, midi);
        for (int row = 0; row < 8; ++row) require(p.getHitCount(row) == before[static_cast<size_t>(row)]+(row == index ? 1u : 0u), "only the selected pattern may generate notes");
    }
    for (int index = 0; index < 4; ++index) p.getPattern(index).setTiming(index, 0, index*25-50);
    set(p, "seqPattern", 2); juce::MemoryBlock state; p.getStateInformation(state);
    RaBubanjProcessor restored; restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    require(restored.getSelectedPattern() == 2, "selected pattern must recall");
    for (int index = 0; index < 4; ++index) require(restored.getPattern(index).snapshot() == p.getPattern(index).snapshot(), "all four patterns must recall independently");
    for (int index = 0; index < 4; ++index) require(restored.getPattern(index).getTiming(index, 0) == index*25-50, "all four patterns must recall independent signed timing offsets");
    auto old = p.parameters.copyState(); old.setProperty("schemaVersion", 5, nullptr);
    for (const auto* id : { "seqEnabled", "seqPattern" }) if (auto child = old.getChildWithProperty("id", id); child.isValid()) old.removeChild(child, nullptr);
    juce::AudioProcessor::copyXmlToBinary(*old.createXml(), state); restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    for (int index = 0; index < 4; ++index) for (auto cell : restored.getPattern(index).snapshot()) require(cell == 0, "pre-sequencer projects must restore four empty patterns");
    old.setProperty("schemaVersion", 6, nullptr); juce::ValueTree legacySequence("SEQUENCER"); juce::String legacyCells;
    for (int i = 0; i < 256; ++i) legacyCells += juce::String(i == 0 ? 1 : i == 33 ? 2 : i == 255 ? 3 : 0);
    legacySequence.setProperty("cells", legacyCells, nullptr); legacySequence.setProperty("brush", 2, nullptr); old.addChild(legacySequence, -1, nullptr);
    juce::AudioProcessor::copyXmlToBinary(*old.createXml(), state); restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    require(restored.getSelectedPattern() == 0 && restored.getPattern(0).get(0, 0) == 2 && restored.getPattern(0).get(1, 1) == 3 && restored.getPattern(0).get(7, 31) == 4,
        "0.7 patterns must migrate into Pattern 1 while preserving their velocity values");
    for (int index = 1; index < 4; ++index) for (auto cell : restored.getPattern(index).snapshot()) require(cell == 0, "legacy migration must leave the other three patterns empty");
    // Schema-7 banks migrate without changing notes and receive zero timing offsets.
    auto seven = p.parameters.copyState(); seven.setProperty("schemaVersion", 7, nullptr); juce::ValueTree banks("SEQUENCER");
    juce::ValueTree bank("PATTERN"); bank.setProperty("index", 2, nullptr); juce::String sevenCells; for (int i = 0; i < 256; ++i) sevenCells += i == 51 ? "3" : "0";
    bank.setProperty("cells", sevenCells, nullptr); banks.addChild(bank, -1, nullptr); seven.addChild(banks, -1, nullptr);
    juce::AudioProcessor::copyXmlToBinary(*seven.createXml(), state); restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    require(restored.getPattern(2).get(1, 19) == 3 && restored.getPattern(2).getTiming(1, 19) == 0, "0.8/0.9 banks must retain notes with neutral timing");
    set(p, "seqPattern", 0); p.getPattern().replace({});
    p.getPattern().set(0, 0, 4); p.getPattern().set(2, 8, 3); p.getPattern().set(4, 16, 2); p.getPattern().set(7, 31, 1);
    set(p, "midiMap", 0); head.bpm = 93; head.playing = false; p.processBlock(audio, midi);
    const auto file = juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("RaBubanj-sequence-test", ".mid", false);
    auto checkFile = [&](const std::array<int, 4>& notes)
    {
        require(p.writeSequenceMidi(file), "MIDI export must write"); auto stream = file.createInputStream(); juce::MidiFile data;
        require(stream && data.readFrom(*stream) && data.getNumTracks() == 1 && data.getTimeFormat() == 960, "export must be a readable single-track PPQ MIDI clip");
        const auto* track = data.getTrack(0); int found = 0; bool tempo = false;
        const std::array<int, 4> times { 0, 1920, 3840, 7440 }, velocities { 127, 88, 48, 32 };
        for (int i = 0; i < track->getNumEvents(); ++i)
        {
            const auto& message = track->getEventPointer(i)->message;
            if (message.isTempoMetaEvent()) tempo = std::abs(message.getTempoSecondsPerQuarterNote()-60.0/93) < .000002;
            if (message.isNoteOn())
            {
                require(found < 4 && message.getNoteNumber() == notes[static_cast<size_t>(found)] && message.getVelocity() == velocities[static_cast<size_t>(found)]
                    && std::abs(message.getTimeStamp()-times[static_cast<size_t>(found)]) < .001 && message.getChannel() == 10, "MIDI file must preserve note, step and velocity"); ++found;
            }
        }
        require(found == 4 && tempo && std::abs(track->getEndTime()-7680) < .001, "MIDI export must retain two-bar length and host tempo metadata");
    };
    checkFile({ 36, 38, 40, 43 }); set(p, "midiMap", 1); checkFile({ 36, 42, 45, 51 });
    p.getPattern(1).replace({}); p.getPattern(1).set(6, 11, 4); set(p, "seqPattern", 1);
    require(p.writeSequenceMidi(file), "alternate pattern must export");
    { auto input = file.createInputStream(); juce::MidiFile data; require(input && data.readFrom(*input), "alternate MIDI file must parse"); int notes = 0;
      for (int i = 0; i < data.getTrack(0)->getNumEvents(); ++i) { const auto& message = data.getTrack(0)->getEventPointer(i)->message; if (message.isNoteOn()) { ++notes; require(message.getNoteNumber() == 49 && std::abs(message.getTimeStamp()-2640) < .001, "MIDI drag must export only the selected pattern"); } }
      require(notes == 1, "unselected patterns must not leak into MIDI export"); }
    p.getPattern(1).setTiming(6, 11, -25); require(p.writeSequenceMidi(file), "shifted pattern must export");
    { auto input = file.createInputStream(); juce::MidiFile data; require(input && data.readFrom(*input), "shifted MIDI must parse");
      for (int i = 0; i < data.getTrack(0)->getNumEvents(); ++i) { const auto& message = data.getTrack(0)->getEventPointer(i)->message; if (message.isNoteOn()) require(message.getTimeStamp() == 2580, "early MIDI note must export sixty ticks earlier"); } }
    p.getPattern(1).replace({}); p.getPattern(1).set(0, 0, 4); p.getPattern(1).setTiming(0, 0, -50);
    require(p.writeSequenceMidi(file), "early first note must export");
    { auto input = file.createInputStream(); juce::MidiFile data; require(input && data.readFrom(*input), "wrapped MIDI must parse");
      for (int i = 0; i < data.getTrack(0)->getNumEvents(); ++i) { const auto& message = data.getTrack(0)->getEventPointer(i)->message; if (message.isNoteOn()) require(message.getTimeStamp() == 7560, "early first note must wrap to the end of the two-bar MIDI clip"); }
      require(data.getTrack(0)->getEndTime() == 7680, "shifted MIDI must keep its eight-beat length"); }
    require(file.deleteFile(), "temporary MIDI must clean up"); set(p, "seqPattern", 0);
    // Verify the actual processor/audio onset, not just the pure timing iterator.
    p.getPattern().replace({}); p.getPattern().set(1, 1, 4); head.bpm = 120; head.playing = true;
    for (int shift : { -40, 40 })
    {
        p.getPattern().setTiming(1, 1, shift); head.ppq = shift < 0 ? .14 : .34;
        p.releaseResources(); p.prepareToPlay(48000, 512); p.processBlock(audio, midi);
        require(audio.getMagnitude(0, 240) == 0 && audio.getMagnitude(241, 271) > .001f, "processor must render early and late notes at their shifted sample offset");
    }
    head.playing = false; p.getPattern().replace({});
    SequencerPanel panel(p); panel.setSize(1160, 620); StepGrid* grid = nullptr;
    for (auto* child : panel.getChildren()) if (auto* g = dynamic_cast<StepGrid*>(child)) grid = g;
    require(grid != nullptr, "step grid must exist"); const auto source = juce::Desktop::getInstance().getMainMouseSource();
    auto event = [&](int row, int step, int mods, bool drag, int elapsed = 0)
    {
        const auto position = grid->cellBounds(row, step).getCentre();
        return juce::MouseEvent(source, position, juce::ModifierKeys(mods), 1, 0, 0, 0, 0, grid, grid, juce::Time::getCurrentTime()+juce::RelativeTime::milliseconds(elapsed), position, juce::Time::getCurrentTime(), 1, drag);
    };
    juce::ComboBox* selector = nullptr;
    for (auto* child : panel.getChildren())
    {
        require(!child->getName().startsWith("Velocity "), "velocity brush buttons must be removed");
        if (auto* box = dynamic_cast<juce::ComboBox*>(child)) selector = box;
    }
    require(selector && selector->getNumItems() == 4, "pattern dropdown must offer four choices");
    selector->setSelectedId(3, juce::sendNotificationSync); require(p.getSelectedPattern() == 2, "dropdown must select the active pattern");
    selector->setSelectedId(1, juce::sendNotificationSync);
    p.getPattern().set(3, 7, 0);
    for (int expected : { 4, 3, 2, 1, 4 })
    {
        grid->mouseDown(event(3, 7, juce::ModifierKeys::leftButtonModifier, false)); grid->mouseUp(event(3, 7, 0, false));
        require(p.getPattern().get(3, 7) == expected, "left click must cycle strongest through weakest and wrap");
    }
    grid->mouseDown(event(3, 7, juce::ModifierKeys::rightButtonModifier, false)); grid->mouseUp(event(3, 7, 0, false)); require(p.getPattern().get(3, 7) == 0, "right click must erase");
    for (int step = 0; step < 32; ++step) p.getPattern().set(5, step, 4);
    grid->mouseDown(event(5, 0, juce::ModifierKeys::rightButtonModifier, false)); grid->mouseDrag(event(5, 31, juce::ModifierKeys::rightButtonModifier, true)); grid->mouseUp(event(5, 31, 0, false));
    for (int step = 0; step < 32; ++step) require(p.getPattern().get(5, step) == 0, "fast right-drag must erase intervening steps without gaps");
    p.getPattern().set(3, 0, 0); p.getPattern().set(3, 31, 0);
    grid->mouseDown(event(3, 0, juce::ModifierKeys::leftButtonModifier, false)); grid->mouseDrag(event(3, 31, juce::ModifierKeys::leftButtonModifier, true)); grid->mouseUp(event(3, 31, 0, false));
    for (int step = 0; step < 32; ++step) require(p.getPattern().get(3, step) == 4, "fast left-drag must fill every intervening cell at maximum strength");
    p.getPattern().set(3, 10, 2); p.getPattern().setTiming(3, 10, -25);
    grid->mouseDown(event(3, 0, juce::ModifierKeys::leftButtonModifier, false)); grid->mouseDrag(event(3, 31, juce::ModifierKeys::leftButtonModifier, true)); grid->mouseDrag(event(3, 0, juce::ModifierKeys::leftButtonModifier, true)); grid->mouseUp(event(3, 0, 0, false));
    require(p.getPattern().get(3, 0) == 4 && p.getPattern().get(3, 10) == 2 && p.getPattern().getTiming(3, 10) == -25, "painting and revisiting cells must preserve existing velocity and timing");
    grid->mouseDown(event(3, 10, juce::ModifierKeys::leftButtonModifier, false)); grid->mouseUp(event(3, 10, 0, false, 700));
    NoteTimingEditor* timingEditor = nullptr; for (auto* child : grid->getChildren()) if (auto* popup = dynamic_cast<NoteTimingEditor*>(child)) timingEditor = popup;
    require(timingEditor && timingEditor->isVisible() && p.getPattern().get(3, 10) == 2, "stationary long press must open timing without cycling velocity");
    juce::Slider* timingSlider = nullptr; for (auto* child : timingEditor->getChildren()) if (auto* slider = dynamic_cast<juce::Slider*>(child)) timingSlider = slider;
    require(timingSlider && timingSlider->getMinimum() == -50 && timingSlider->getMaximum() == 50, "timing slider must support earlier and later half-step offsets");
    timingSlider->setValue(35, juce::sendNotificationSync); require(p.getPattern().getTiming(3, 10) == 35 && p.getPattern().get(3, 10) == 2, "slider must change only the held note timing");
    set(p, "seqPattern", 1); grid->refresh(); for (auto* child : grid->getChildren()) require(dynamic_cast<NoteTimingEditor*>(child) == nullptr, "timing popup must close when the selected pattern changes");
    require(p.getPattern(1).getTiming(3, 10) == 0, "timing edits must not leak to other banks"); set(p, "seqPattern", 0);
    for (int step = 0; step < 32; ++step) p.getPattern(0).set(4, step, 4);
    p.getPattern(1).set(4, 31, 4);
    grid->mouseDown(event(4, 0, juce::ModifierKeys::rightButtonModifier, false)); set(p, "seqPattern", 1);
    grid->mouseDrag(event(4, 31, juce::ModifierKeys::rightButtonModifier, true)); grid->mouseUp(event(4, 31, 0, false));
    require(p.getPattern(0).get(4, 31) == 0 && p.getPattern(1).get(4, 31) == 4, "a drag gesture must remain bound to its original pattern");
    set(p, "seqPattern", 0);
    // PAD labels select and audition exactly once without modifying any pattern cell.
    const auto patternBeforePad = p.getPattern().snapshot(); p.getPattern().replace({});
    for (int row = 0; row < 8; ++row)
    {
        const auto before = p.getHitCount(row); const auto point = grid->padBounds(row).getCentre();
        const juce::MouseEvent click(source, point, juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier), 1, 0, 0, 0, 0, grid, grid, juce::Time::getCurrentTime(), point, juce::Time::getCurrentTime(), 1, false);
        grid->mouseDown(click); grid->mouseUp(click); p.processBlock(audio, midi);
        require(p.getSelectedChannel() == row && grid->getSelectedPad() == row && p.getHitCount(row) == before+1, "PAD label must select and audition its corresponding sample");
        for (auto cell : p.getPattern().snapshot()) require(cell == 0, "PAD labels must not write sequence steps");
    }
    p.getPattern().replace(patternBeforePad);
    // The scope reads actual stereo audio, discards obsolete pad generations and clears on selection.
    PadWaveform scope(p); scope.setSize(1100, 128); scope.update(0, true);
    p.releaseResources(); p.prepareToPlay(48000, 512); p.audition(0); p.processBlock(audio, midi); scope.update(0, true);
    require(scope.getDisplayedPeak() > .01f, "selected-pad waveform must receive real engine audio");
    scope.update(1, true); require(scope.getDisplayedPeak() == 0, "switching pads must clear the previous waveform");
    p.processBlock(audio, midi); scope.update(1, true); require(scope.getDisplayedPeak() == 0, "unselected pad tails must not leak into the selected waveform");
    p.audition(1); p.processBlock(audio, midi); scope.update(1, true); require(scope.getDisplayedPeak() > .01f, "newly selected pad must display its own sound");
    scope.update(1, false); require(rabubanj::WaveformTap::padForToken(p.getWaveformTap().getToken()) == -1, "hidden scope must disable audio capture");
    {
        rabubanj::WaveformTap tap; tap.selectPad(2); const auto token = tap.getToken();
        std::array<float, 240> l, r; l.fill(.5f); r.fill(-.5f); tap.push(l.data(), r.data(), 240, token, 48000);
        std::array<rabubanj::WaveformTap::Frame, 8192> data; const auto count = tap.read(data.data(), 8192);
        require(count == 10 && data[0].low == -.5f && data[0].high == .5f && data[0].token == token, "scope must preserve anti-phase stereo peaks and calibrated time scale");
        for (int i = 0; i < 1000; ++i) tap.push(l.data(), r.data(), 240, token, 48000);
        require(tap.read(data.data(), 8192) == 8191, "scope queue must remain bounded when GUI stops consuming");
        tap.selectPad(3); tap.selectPad(2); require(tap.getToken() != token, "returning to a pad must use a new generation to reject stale audio");
    }
    std::unique_ptr<RaBubanjEditor> editor(static_cast<RaBubanjEditor*>(p.createEditor()));
    editor->setVisible(true); editor->showSequencer();
    SequencerWindow* window = nullptr;
    for (auto* child : editor->getChildren()) if (auto* w = dynamic_cast<SequencerWindow*>(child)) window = w;
    require(window && window->isVisible() && !window->isOnDesktop(), "sequencer must open inside the host plugin view");
    auto checkMixerVisibility = [&](bool visible)
    {
        for (auto* child : editor->getChildren())
            if (dynamic_cast<MixerStrip*>(child) || dynamic_cast<EqPanel*>(child) || dynamic_cast<juce::Slider*>(child))
                require(child->isVisible() == visible, "switching pages must hide or restore all mixer controls and EQ panels");
    };
    checkMixerVisibility(false);
    require(!editor->keyPressed(juce::KeyPress(juce::KeyPress::spaceKey)) && !window->keyPressed(juce::KeyPress(juce::KeyPress::spaceKey)), "Space must remain unhandled for the DAW transport");
    for (auto size : { juce::Point<int>(1120, 740), juce::Point<int>(1280, 820), juce::Point<int>(1800, 1160) })
    {
        editor->setSize(size.x, size.y);
        require(editor->getLocalBounds().contains(window->getBounds()) && window->getWidth() >= editor->getWidth()-50 && window->getHeight() >= editor->getHeight()-100,
            "sequencer page must fill the working area at every supported plugin size");
        checkMixerVisibility(false);
    }
    auto* close = window->getCloseButton();
    require(close && window->getComponentAt(close->getBounds().getCentre()) == close && close->onClick, "close control must be an unobstructed, directly wired button");
    p.getWaveformTap().selectPad(0); close->onClick(); require(!window->isVisible(), "clicking the close button must hide the sequencer");
    checkMixerVisibility(true);
    require(rabubanj::WaveformTap::padForToken(p.getWaveformTap().getToken()) == -1, "closing the window must immediately disable its waveform capture");
    juce::TextButton* toggle = nullptr;
    for (auto* child : editor->getChildren()) if (auto* button = dynamic_cast<juce::TextButton*>(child); button && button->getName() == "Open step sequencer") toggle = button;
    require(toggle && toggle->onClick, "main STEP SEQ toggle must exist");
    toggle->onClick(); require(window->isVisible() && toggle->getButtonText() == "MIXER", "STEP SEQ must reopen the sequencer and offer a return to MIXER");
    checkMixerVisibility(false);
    require(editor->getComponentAt(toggle->getBounds().getCentre()) == toggle, "page navigation must remain accessible above the sequencer");
    toggle->onClick(); require(!window->isVisible() && toggle->getButtonText() == "STEP SEQ", "MIXER must restore the mixer and the STEP SEQ caption");
    checkMixerVisibility(true);
    editor->showSequencer(); window->setLookAndFeel(&editor->getLookAndFeel());
    window->getCloseButton()->onClick(); require(!window->isVisible(), "close action must survive look-and-feel changes");
    checkMixerVisibility(true);
    if (argc > 1)
    {
        set(p, "midiMap", 0);
        p.getPattern().replace({});
        for (int step : { 0, 6, 16, 22 }) p.getPattern().set(0, step, 4);
        for (int step : { 8, 24 }) p.getPattern().set(1, step, 4);
        for (int step = 0; step < 32; step += 2) p.getPattern().set(2, step, step%4 ? 2 : 3);
        p.getPattern().set(3, 15, 2); p.getPattern().set(3, 31, 1); panel.refresh();
        juce::File directory(argv[1]); directory.createDirectory(); juce::PNGImageFormat png;
        auto save = [&](juce::Component& component, const char* name)
        { auto output = directory.getChildFile(name).createOutputStream(); if (output) { output->setPosition(0); output->truncate(); } require(output && png.writeImageToStream(component.createComponentSnapshot(component.getLocalBounds()), *output), "sequencer preview must write"); };
        // Feed a real preview hit through the same capture path as sequence playback.
        for (auto* child : panel.getChildren()) if (auto* wave = dynamic_cast<PadWaveform*>(child))
        { wave->update(grid->getSelectedPad(), true); p.audition(grid->getSelectedPad()); for (int block = 0; block < 30; ++block) { p.processBlock(audio, midi); wave->update(grid->getSelectedPad(), true); } }
        panel.setLookAndFeel(&editor->getLookAndFeel());
        save(panel, "RaBubanj-sequencer.png"); editor->setSize(1280, 820); editor->showSequencer();
        for (auto* child : window->getChildren()) if (auto* contents = dynamic_cast<SequencerPanel*>(child))
            for (auto* component : contents->getChildren()) if (auto* wave = dynamic_cast<PadWaveform*>(component))
            { wave->update(0, true); p.audition(0); for (int block = 0; block < 30; ++block) { p.processBlock(audio, midi); wave->update(0, true); } }
        save(*editor, "RaBubanj-sequencer-window.png");
        for (auto size : { juce::Point<int>(1120, 740), juce::Point<int>(1800, 1160) })
        {
            editor->setSize(size.x, size.y);
            save(*editor, ("RaBubanj-sequencer-"+juce::String(size.x)+".png").toRawUTF8());
        }
        grid->mouseDown(event(1, 8, juce::ModifierKeys::leftButtonModifier, false)); grid->mouseUp(event(1, 8, 0, false, 700));
        for (auto* child : grid->getChildren()) if (auto* popup = dynamic_cast<NoteTimingEditor*>(child)) for (auto* control : popup->getChildren()) if (auto* slider = dynamic_cast<juce::Slider*>(control)) slider->setValue(20, juce::sendNotificationSync);
        save(panel, "RaBubanj-note-timing.png");
        panel.setLookAndFeel(nullptr);
    }
    std::cout << "Host transport, exact note offsets, panic, pattern recall, MIDI export, four-pattern recall/migration, click cycling, left-drag paint, long-press timing, shifted audio/MIDI, right-drag erase, child window and Space forwarding passed.\n";
}
