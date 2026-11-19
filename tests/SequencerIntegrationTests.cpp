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
    p.getPattern().set(0, 0, 3); head.ppq = -.01;
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
    p.setSequenceBrush(2); juce::MemoryBlock state; p.getStateInformation(state);
    RaBubanjProcessor restored; restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    require(restored.getPattern().get(0, 0) == 3 && restored.getPattern().get(1, 0) == 2 && restored.getSequenceBrush() == 2, "pattern and velocity brush must recall in a DAW project");
    auto old = p.parameters.copyState(); old.setProperty("schemaVersion", 5, nullptr);
    if (auto child = old.getChildWithProperty("id", "seqEnabled"); child.isValid()) old.removeChild(child, nullptr);
    juce::AudioProcessor::copyXmlToBinary(*old.createXml(), state); restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    for (auto cell : restored.getPattern().snapshot()) require(cell == 0, "old projects must restore an empty pattern");
    p.getPattern().replace({}); p.getPattern().set(0, 0, 1); p.getPattern().set(2, 15, 2); p.getPattern().set(7, 31, 3);
    set(p, "midiMap", 0); head.bpm = 93; head.playing = false; p.processBlock(audio, midi);
    const auto file = juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("RaBubanj-sequence-test", ".mid", false);
    auto checkFile = [&](const std::array<int, 3>& notes)
    {
        require(p.writeSequenceMidi(file), "MIDI export must write"); auto stream = file.createInputStream(); juce::MidiFile data;
        require(stream && data.readFrom(*stream) && data.getNumTracks() == 1 && data.getTimeFormat() == 960, "export must be a readable single-track PPQ MIDI clip");
        const auto* track = data.getTrack(0); int found = 0; bool tempo = false;
        const std::array<int, 3> times { 0, 3600, 7440 }, velocities { 48, 88, 127 };
        for (int i = 0; i < track->getNumEvents(); ++i)
        {
            const auto& message = track->getEventPointer(i)->message;
            if (message.isTempoMetaEvent()) tempo = std::abs(message.getTempoSecondsPerQuarterNote()-60.0/93) < .000002;
            if (message.isNoteOn())
            {
                require(found < 3 && message.getNoteNumber() == notes[static_cast<size_t>(found)] && message.getVelocity() == velocities[static_cast<size_t>(found)]
                    && std::abs(message.getTimeStamp()-times[static_cast<size_t>(found)]) < .001 && message.getChannel() == 10, "MIDI file must preserve note, step and velocity"); ++found;
            }
        }
        require(found == 3 && tempo && std::abs(track->getEndTime()-7680) < .001, "MIDI export must retain two-bar length and host tempo metadata");
    };
    checkFile({ 36, 38, 43 }); set(p, "midiMap", 1); checkFile({ 36, 42, 51 }); require(file.deleteFile(), "temporary MIDI must clean up");
    SequencerPanel panel(p); panel.setSize(1040, 440); StepGrid* grid = nullptr;
    for (auto* child : panel.getChildren()) if (auto* g = dynamic_cast<StepGrid*>(child)) grid = g;
    require(grid != nullptr, "step grid must exist"); const auto source = juce::Desktop::getInstance().getMainMouseSource();
    auto event = [&](int row, int step, int mods, bool drag)
    {
        const auto position = grid->cellBounds(row, step).getCentre();
        return juce::MouseEvent(source, position, juce::ModifierKeys(mods), 1, 0, 0, 0, 0, grid, grid, juce::Time::getCurrentTime(), position, juce::Time::getCurrentTime(), 1, drag);
    };
    for (int strength = 1; strength <= 3; ++strength)
    {
        p.setSequenceBrush(strength); grid->mouseDown(event(3, 7, juce::ModifierKeys::rightButtonModifier, false)); grid->mouseUp(event(3, 7, 0, false));
        require(p.getPattern().get(3, 7) == strength, "right click must write the selected velocity");
    }
    grid->mouseDown(event(3, 7, juce::ModifierKeys::leftButtonModifier, false)); grid->mouseUp(event(3, 7, 0, false)); require(p.getPattern().get(3, 7) == 0, "left click must erase");
    for (int step = 0; step < 32; ++step) p.getPattern().set(5, step, 3);
    grid->mouseDown(event(5, 0, juce::ModifierKeys::leftButtonModifier, false)); grid->mouseDrag(event(5, 31, juce::ModifierKeys::leftButtonModifier, true)); grid->mouseUp(event(5, 31, 0, false));
    for (int step = 0; step < 32; ++step) require(p.getPattern().get(5, step) == 0, "fast left-drag must erase intervening steps without gaps");
    std::unique_ptr<RaBubanjEditor> editor(static_cast<RaBubanjEditor*>(p.createEditor())); editor->showSequencer();
    SequencerWindow* window = nullptr;
    for (auto* child : editor->getChildren()) if (auto* w = dynamic_cast<SequencerWindow*>(child)) window = w;
    require(window && window->isVisible() && !window->isOnDesktop(), "sequencer must open as a child window within the host plugin view");
    require(!editor->keyPressed(juce::KeyPress(juce::KeyPress::spaceKey)) && !window->keyPressed(juce::KeyPress(juce::KeyPress::spaceKey)), "Space must remain unhandled for the DAW transport");
    for (auto size : { juce::Point<int>(1120, 740), juce::Point<int>(1280, 820), juce::Point<int>(1800, 1160) })
    { editor->setSize(size.x, size.y); require(editor->getLocalBounds().contains(window->getBounds()), "sequencer window must remain inside resized plugin bounds"); }
    window->closeButtonPressed(); require(!window->isVisible(), "closing the sequencer window must hide it");
    if (argc > 1)
    {
        set(p, "midiMap", 0);
        p.getPattern().replace({});
        for (int step : { 0, 6, 16, 22 }) p.getPattern().set(0, step, 3);
        for (int step : { 8, 24 }) p.getPattern().set(1, step, 3);
        for (int step = 0; step < 32; step += 2) p.getPattern().set(2, step, step%4 ? 1 : 2);
        p.getPattern().set(3, 15, 2); p.getPattern().set(3, 31, 2); panel.refresh();
        juce::File directory(argv[1]); directory.createDirectory(); juce::PNGImageFormat png;
        auto save = [&](juce::Component& component, const char* name)
        { auto output = directory.getChildFile(name).createOutputStream(); if (output) { output->setPosition(0); output->truncate(); } require(output && png.writeImageToStream(component.createComponentSnapshot(component.getLocalBounds()), *output), "sequencer preview must write"); };
        save(panel, "RaBubanj-sequencer.png"); editor->setSize(1280, 820); editor->showSequencer(); save(*editor, "RaBubanj-sequencer-window.png");
    }
    std::cout << "Host transport, exact note offsets, panic, pattern recall, MIDI export, velocity writing, drag erase, child window and Space forwarding passed.\n";
}
