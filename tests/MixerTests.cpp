#include "PluginEditor.h"
#include <cstdlib>
#include <iostream>
static void require(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
static void set(RaBubanjProcessor& p, const juce::String& id, float value)
{
    auto* parameter = p.parameters.getParameter(id); require(parameter != nullptr, "parameter must exist");
    parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
}
static double render(RaBubanjProcessor& p)
{
    p.setRateAndBufferSizeDetails(48000, 512); p.prepareToPlay(48000, 512);
    juce::AudioBuffer<float> audio(2, 512); juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, 36, .8f), 0);
    double energy = 0;
    for (int block = 0; block < 130; ++block)
    {
        p.processBlock(audio, midi);
        for (int c = 0; c < 2; ++c) for (int s = 0; s < 512; ++s)
        {
            const auto sample = audio.getSample(c, s); require(std::isfinite(sample), "mixer audio must be finite");
            energy += sample*sample;
        }
    }
    return energy;
}
static void checkBounds(juce::Component& component)
{
    for (auto* child : component.getChildren()) if (child->isVisible())
    {
        require(component.getLocalBounds().contains(child->getBounds()), "visible control must fit its parent");
        if (dynamic_cast<MixerStrip*>(child) || dynamic_cast<EqPanel*>(child)) checkBounds(*child);
    }
}
int main(int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    RaBubanjProcessor p;
    set(p, "solo8", 1); set(p, "room", .8f); set(p, "send0", 0);
    require(render(p) < 1e-12, "zero send must leave the wet return silent");
    set(p, "send0", 1); const auto wetEnergy = render(p);
    require(wetEnergy > .1, "post-fader send must reach the Reverb bus");
    set(p, "gain0", -24); const auto quietWetEnergy = render(p);
    require(quietWetEnergy < wetEnergy*.01 && quietWetEnergy > 0, "channel fader must also reduce its post-fader send");
    set(p, "gain0", 0); set(p, "mute0", 1);
    require(render(p) < 1e-12, "muted drum must not feed the reverb");
    set(p, "mute0", 0); set(p, "reverbMute", 1);
    require(render(p) < 1e-12, "reverb mute must silence the wet return");
    set(p, "reverbMute", 0); set(p, "room", 0);
    require(render(p) < 1e-12, "reverb fader at zero must silence the return");
    set(p, "room", .8f); set(p, "solo8", 0); set(p, "solo1", 1);
    require(render(p) < 1e-12, "soloing snare must exclude kick from dry and send paths");
    set(p, "solo1", 0); set(p, "send0", 0); set(p, "room", 0);
    const auto flatKick = render(p);
    set(p, "eq1b0gain", 18); // A snare EQ edit cannot affect the kick.
    require(std::abs(render(p)-flatKick) < 1e-8, "per-channel EQ must isolate instruments");
    set(p, "eq0b0gain", 18); const auto boostedKick = render(p);
    require(boostedKick > flatKick*1.8, "kick EQ must actually process the kick audio");
    set(p, "eqBypass0", 1);
    require(std::abs(render(p)-flatKick) < .001, "channel EQ bypass must restore unfiltered audio");
    set(p, "eq9b0gain", -18);
    require(render(p) < flatKick*.7, "global EQ must process the final mix");
    set(p, "eq9b0gain", 0); set(p, "eqBypass0", 0); set(p, "solo8", 1); set(p, "send0", 1); set(p, "room", .8f);
    const auto flatReturn = render(p);
    set(p, "eq8b0gain", -18);
    require(render(p) < flatReturn*.8, "Reverb return EQ must process wet audio");

    p.selectChannel(5); set(p, "send5", .731f); set(p, "eq5b2freq", 3271); set(p, "eq5b2gain", -7.3f);
    set(p, "eq5b2q", 4.1f); set(p, "eq5b2type", 5); set(p, "eq5b2on", 0); set(p, "roomSize", .71f);
    juce::MemoryBlock state; p.getStateInformation(state);
    RaBubanjProcessor restored; restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    for (const auto* id : { "send5", "eq5b2freq", "eq5b2gain", "eq5b2q", "eq5b2type", "eq5b2on", "roomSize", "room", "eq8b0gain", "eq9b0gain" })
        require(std::abs(p.parameters.getRawParameterValue(id)->load() - restored.parameters.getRawParameterValue(id)->load()) < .001f, "new mixer / EQ values must recall in DAW state");
    require(restored.getSelectedChannel() == 5, "selected channel must survive project recall");

    juce::ValueTree legacy("RaBubanjState");
    for (const auto& item : { std::pair<const char*, float>("gain0", -9), { "room", .31f } })
    {
        juce::ValueTree value("PARAM"); value.setProperty("id", item.first, nullptr); value.setProperty("value", item.second, nullptr); legacy.addChild(value, -1, nullptr);
    }
    juce::AudioProcessor::copyXmlToBinary(*legacy.createXml(), state);
    restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    require(std::abs(restored.parameters.getRawParameterValue("gain0")->load()+9) < .001f, "0.1 mixer levels must migrate");
    require(restored.parameters.getRawParameterValue("send0")->load() > .999f, "0.1 projects must migrate with unity room sends");
    require(std::abs(restored.parameters.getRawParameterValue("eq5b2gain")->load()) < .001f, "old projects must reset absent EQs to neutral");

    RaBubanjProcessor uiProcessor;
    std::unique_ptr<RaBubanjEditor> editor(static_cast<RaBubanjEditor*>(uiProcessor.createEditor()));
    EqPanel* channelEq = nullptr; EqPanel* globalEq = nullptr;
    for (auto* child : editor->getChildren()) if (auto* panel = dynamic_cast<EqPanel*>(child))
    { if (panel->getBus() == 9) globalEq = panel; else channelEq = panel; }
    require(channelEq && globalEq, "both channel and global EQ must be visible");
    for (int channel = 0; channel < 9; ++channel)
    {
        juce::TextButton* select = nullptr;
        const auto name = channel < 8 ? juce::String(rabubanj::names[static_cast<size_t>(channel)]) : "Reverb";
        for (auto* child : editor->getChildren()) if (auto* strip = dynamic_cast<MixerStrip*>(child))
            for (auto* control : strip->getChildren()) if (control->getName() == "Select " + name) select = dynamic_cast<juce::TextButton*>(control);
        require(select != nullptr, "channel selection control must exist"); select->onClick();
        require(channelEq->getBus() == channel && globalEq->getBus() == 9, "clicking a channel must rebind only the channel EQ");
    }
    editor->selectChannel(0); channelEq->selectBand(2);
    // Exercise the attachment after rebinding from the Reverb bus to Kick.
    juce::Slider* eqGain = nullptr;
    for (auto* child : channelEq->getChildren()) if (child->getName() == "EQ gain") eqGain = dynamic_cast<juce::Slider*>(child);
    require(eqGain != nullptr, "EQ gain control must exist"); eqGain->setValue(6, juce::sendNotificationSync);
    require(std::abs(uiProcessor.parameters.getRawParameterValue("eq0b2gain")->load()-6) < .01f, "EQ control must edit the selected channel and band");
    require(std::abs(uiProcessor.parameters.getRawParameterValue("eq8b2gain")->load()) < .001f, "previous channel must keep its own EQ values");
    juce::Component* graph = nullptr;
    for (auto* child : channelEq->getChildren()) if (child->getName() == "EQ response graph") graph = child;
    require(graph != nullptr, "interactive EQ graph must exist");
    const auto source = juce::Desktop::getInstance().getMainMouseSource();
    const auto centre = graph->getLocalBounds().toFloat().getCentre();
    auto event = [&](juce::Point<float> position, bool down, bool drag)
    {
        return juce::MouseEvent(source, position, juce::ModifierKeys(down ? juce::ModifierKeys::leftButtonModifier : 0),
            1, 0, 0, 0, 0, graph, graph, juce::Time::getCurrentTime(), centre, juce::Time::getCurrentTime(), 1, drag);
    };
    const auto originalFreq = uiProcessor.parameters.getRawParameterValue("eq0b2freq")->load();
    graph->mouseDown(event(centre, true, false));
    graph->mouseDrag(event(centre.translated(45, -20), true, true));
    graph->mouseUp(event(centre.translated(45, -20), false, true));
    require(std::abs(uiProcessor.parameters.getRawParameterValue("eq0b2freq")->load()-originalFreq) > 10,
        "dragging an EQ node must change frequency");
    require(uiProcessor.parameters.getRawParameterValue("eq0b2gain")->load() > 0, "dragging an EQ node vertically must change gain");
    const auto originalQ = uiProcessor.parameters.getRawParameterValue("eq0b2q")->load();
    juce::MouseWheelDetails wheel { 0, .2f, false, false, false };
    graph->mouseWheelMove(event(centre.translated(45, -20), false, false), wheel);
    require(uiProcessor.parameters.getRawParameterValue("eq0b2q")->load() > originalQ, "EQ graph wheel must change Q");
    graph->mouseDoubleClick(event(centre.translated(45, -20), false, false));
    require(std::abs(uiProcessor.parameters.getRawParameterValue("eq0b2gain")->load()) < .001f, "double click must reset band gain");
    set(uiProcessor, "eq0b2gain", 6); channelEq->refresh();
    for (auto* child : editor->getChildren()) if (auto* strip = dynamic_cast<MixerStrip*>(child))
        for (auto* control : strip->getChildren()) if (auto* s = dynamic_cast<juce::Slider*>(control))
        {
            if (s->getName() == "Kick reverb send") require(s->getTextFromValue(.25) == "25%", "Send must display and parse percent units");
            if (s->getName() == "Reverb level")
            {
                require(s->getTextFromValue(.1) == "-20.0", "Reverb return fader must display dB");
                for (auto* labelComponent : s->getChildren()) if (auto* label = dynamic_cast<juce::Label*>(labelComponent))
                    require(label->getText() == s->getTextFromValue(s->getValue()), "visible return value must use dB formatting");
            }
        }
    for (auto size : { juce::Point<int>(1120, 740), juce::Point<int>(1280, 820), juce::Point<int>(1800, 1160) })
    {
        editor->setSize(size.x, size.y); checkBounds(*editor);
        if (argc > 1)
        {
            juce::File file(juce::String(argv[1]) + "/RaBubanj-" + juce::String(size.x) + ".png");
            file.getParentDirectory().createDirectory(); auto stream = file.createOutputStream(); juce::PNGImageFormat png;
            if (stream) { stream->setPosition(0); stream->truncate(); }
            require(stream && png.writeImageToStream(editor->createComponentSnapshot(editor->getLocalBounds()), *stream), "UI snapshot must be written");
        }
    }
    std::cout << "Send routing, post-fader / mute / solo, channel / return / master EQ, state migration and editor selection / sizing passed.\n";
}
