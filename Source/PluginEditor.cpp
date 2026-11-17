#include "PluginEditor.h"
using namespace rabubanj::ui;

MixerLookAndFeel::MixerLookAndFeel()
{
    setColour(juce::TextButton::buttonColourId, juce::Colour(0xff353d45));
    setColour(juce::TextButton::buttonOnColourId, cyan);
    setColour(juce::TextButton::textColourOffId, ink); setColour(juce::TextButton::textColourOnId, dark);
    setColour(juce::Slider::backgroundColourId, dark); setColour(juce::Slider::thumbColourId, cyan);
    setColour(juce::ComboBox::backgroundColourId, dark); setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff44505a));
    setColour(juce::ComboBox::textColourId, ink); setColour(juce::PopupMenu::backgroundColourId, panel);
    setColour(juce::PopupMenu::textColourId, ink); setColour(juce::ToggleButton::textColourId, muted);
    setColour(juce::ToggleButton::tickColourId, cyan);
}
void MixerLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float position, float minimum, float maximum, juce::Slider::SliderStyle style, juce::Slider& s)
{
    if (style != juce::Slider::LinearVertical && style != juce::Slider::LinearHorizontal)
    { LookAndFeel_V4::drawLinearSlider(g, x, y, w, h, position, minimum, maximum, style, s); return; }
    const auto colour = s.findColour(juce::Slider::trackColourId);
    if (style == juce::Slider::LinearVertical)
    {
        const float cx = static_cast<float>(x) + static_cast<float>(w)*.5f;
        g.setColour(dark); g.fillRoundedRectangle(cx-3, static_cast<float>(y), 6, static_cast<float>(h), 2);
        g.setColour(colour.withAlpha(.5f)); g.fillRect(cx-1, position, 2.0f, static_cast<float>(y+h)-position);
        juce::Rectangle<float> thumb(cx-8, position-5, 16, 10);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xffd0d7dc), cx, position-5, juce::Colour(0xff65727f), cx, position+5, false));
        g.fillRoundedRectangle(thumb, 2); g.setColour(juce::Colour(0xff252b31)); g.drawHorizontalLine(juce::roundToInt(position), cx-6, cx+6);
    }
    else
    {
        const auto cy = static_cast<float>(y) + static_cast<float>(h)*.5f;
        g.setColour(dark); g.fillRoundedRectangle(static_cast<float>(x), cy-2, static_cast<float>(w), 4, 2);
        g.setColour(colour); g.fillRoundedRectangle(position-3, cy-5, 6, 10, 1);
    }
}

MixerStrip::MixerStrip(RaBubanjProcessor& p, int busToUse) : processor(p), bus(busToUse),
    channelName(bus < 8 ? rabubanj::names[static_cast<size_t>(bus)] : bus == 8 ? "Reverb" : "Master")
{
    setName(channelName + " channel"); addMouseListener(this, true);
    select.setButtonText(channelName); select.setName("Select " + channelName);
    select.setColour(juce::TextButton::buttonOnColourId, bus >= 8 ? gold : cyan);
    select.onClick = [this] { if (onSelect) onSelect(bus); };
    eq.setName("Open " + channelName + " EQ"); eq.onClick = select.onClick;
    select.setTooltip("Select " + channelName + " and open its EQ"); eq.setTooltip("Open " + channelName + " parametric EQ");
    for (auto* b : { &select, &eq, &mute }) addAndMakeVisible(*b);
    mute.setClickingTogglesState(true); mute.setTooltip("Mute " + channelName);
    mute.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffb67a62));
    slider(gain, juce::Slider::LinearVertical, 46); gain.setName(channelName + " level");
    gain.setNumDecimalPlacesToDisplay(1); addAndMakeVisible(gain);
    const auto id = juce::String(bus);
    if (bus < 8)
    {
        gain.setDoubleClickReturnValue(true, 0);
        slider(pan, juce::Slider::LinearHorizontal); pan.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        pan.setDoubleClickReturnValue(true, 0); pan.setName(channelName + " pan"); pan.setTooltip("Pan: left / centre / right"); addAndMakeVisible(pan);
        slider(send, juce::Slider::LinearVertical, 36); send.setName(channelName + " reverb send");
        send.setColour(juce::Slider::trackColourId, gold); send.setDoubleClickReturnValue(true, .25);
        send.setTooltip("Post-fader send to the shared Reverb bus"); addAndMakeVisible(send);
        gainAttachment = std::make_unique<SliderAttachment>(p.parameters, "gain" + id, gain);
        panAttachment = std::make_unique<SliderAttachment>(p.parameters, "pan" + id, pan);
        sendAttachment = std::make_unique<SliderAttachment>(p.parameters, "send" + id, send);
        muteAttachment = std::make_unique<ButtonAttachment>(p.parameters, "mute" + id, mute);
        audition.setButtonText(juce::String(rabubanj::notes[static_cast<size_t>(bus)]) + "  HIT"); audition.setName("Preview " + channelName);
        audition.onClick = [this] { processor.audition(bus); }; addAndMakeVisible(audition);
    }
    else
    {
        gain.setDoubleClickReturnValue(true, bus == 8 ? .12 : -8);
        gainAttachment = std::make_unique<SliderAttachment>(p.parameters, bus == 8 ? "room" : "master", gain);
        muteAttachment = std::make_unique<ButtonAttachment>(p.parameters, bus == 8 ? "reverbMute" : "masterMute", mute);
    }
    if (bus < 9)
    {
        solo.setClickingTogglesState(true); solo.setTooltip(bus == 8 ? "Listen to the wet Reverb return only" : "Solo " + channelName);
        solo.setColour(juce::TextButton::buttonOnColourId, gold); addAndMakeVisible(solo);
        soloAttachment = std::make_unique<ButtonAttachment>(p.parameters, "solo" + id, solo);
    }
    // APVTS attachments install parameter text callbacks, so set our display units afterwards.
    if (bus < 8)
    {
        send.textFromValueFunction = [](double v) { return juce::String(juce::roundToInt(v*100)) + "%"; };
        send.valueFromTextFunction = [](const juce::String& v) { return v.getDoubleValue()*.01; };
        send.updateText();
    }
    if (bus == 8)
    {
        gain.textFromValueFunction = [](double v) { return v < .00005 ? juce::String("-inf") : juce::String(20*std::log10(v), 1); };
        gain.valueFromTextFunction = [](const juce::String& v) { return v.containsIgnoreCase("inf") ? 0.0 : std::pow(10.0, v.getDoubleValue()/20); };
        gain.updateText();
    }
    refresh();
}
void MixerStrip::mouseDown(const juce::MouseEvent&) { if (onSelect) onSelect(bus); }
void MixerStrip::resized()
{
    auto area = getLocalBounds().reduced(6, 8);
    select.setBounds(area.removeFromTop(25)); area.removeFromTop(5);
    eq.setBounds(area.removeFromTop(19)); area.removeFromTop(5);
    auto bottom = area.removeFromBottom(56);
    auto buttons = bottom.removeFromTop(21);
    mute.setBounds(buttons.removeFromLeft(buttons.getWidth()/2).reduced(1, 0)); solo.setBounds(buttons.reduced(1, 0));
    bottom.removeFromTop(7); audition.setBounds(bottom.removeFromTop(21));
    auto panArea = area.removeFromTop(27); pan.setBounds(panArea.withTrimmedTop(9));
    area.removeFromBottom(6); area.removeFromTop(15); faderArea = area;
    if (bus < 8)
    {
        auto volume = area.removeFromLeft(area.getWidth()-36); gain.setBounds(volume.withTrimmedRight(4));
        meterArea = volume.removeFromRight(4).withTrimmedBottom(23);
        send.setBounds(area);
    }
    else
    {
        gain.setBounds(area.withTrimmedRight(16)); meterArea = area.removeFromRight(9).withTrimmedBottom(23);
    }
    gain.updateText(); send.updateText();
}
void MixerStrip::paint(juce::Graphics& g)
{
    const bool selected = bus == processor.getSelectedChannel() || bus == 9;
    const auto colour = bus >= 8 ? gold : cyan;
    g.setColour(selected ? panel.brighter(.08f) : panel); g.fillRoundedRectangle(getLocalBounds().toFloat(), 3);
    g.setColour(selected ? colour : colour.darker(.5f)); g.fillRect(0, 0, getWidth(), 3);
    g.setColour(juce::Colour(0xff4b555f)); g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(.5f), 3, .7f);
    text(g, bus < 8 ? "PAN" : bus == 8 ? "WET RETURN" : "STEREO OUT", { 8, 63, getWidth()-16, 13 }, 8, muted, juce::Justification::centred);
    text(g, "dB", { gain.getX(), faderArea.getY()-15, gain.getWidth(), 13 }, 9, muted, juce::Justification::centred);
    if (bus < 8) text(g, "SEND", { send.getX(), faderArea.getY()-15, send.getWidth(), 13 }, 8, gold, juce::Justification::centred);
    g.setColour(dark); g.fillRect(meterArea);
    const auto db = juce::Decibels::gainToDecibels(processor.getPeak(bus), -60.0f);
    const auto amount = juce::jlimit(0.0f, 1.0f, (db+60)/60);
    auto meter = meterArea.toFloat(); meter.removeFromTop(meter.getHeight()*(1-amount));
    g.setColour(db > -1 ? juce::Colour(0xffdc8875) : colour); g.fillRect(meter);
    if (bus < 8)
    {
        g.setColour(colour.withAlpha(.12f + flash*.88f)); g.fillRect(8, getHeight()-5, getWidth()-16, 2);
    }
    else text(g, bus == 8 ? "POST SENDS" : "OUT 1 / 2", { 5, getHeight()-32, getWidth()-10, 18 }, 8, muted, juce::Justification::centred);
}
void MixerStrip::refresh()
{
    if (bus < 8)
    {
        const auto count = processor.getHitCount(bus); flash = count != lastHit ? 1.0f : flash*.76f; lastHit = count;
    }
    select.setToggleState(bus == 9 || bus == processor.getSelectedChannel(), juce::dontSendNotification);
    const bool bypassed = processor.getEqSettings(bus).bypass;
    eq.setButtonText(bypassed ? "EQ BYPASS" : "EQ"); eq.setColour(juce::TextButton::textColourOffId, bypassed ? muted : cyan);
    repaint();
}

RaBubanjEditor::RaBubanjEditor(RaBubanjProcessor& p)
    : AudioProcessorEditor(&p), processor(p), channelEq(p, p.getSelectedChannel()), globalEq(p, rabubanj::masterBus, true)
{
    setLookAndFeel(&look); addAndMakeVisible(channelEq); addAndMakeVisible(globalEq);
    for (int i = 0; i < rabubanj::eqBusCount; ++i)
    {
        strips[static_cast<size_t>(i)] = std::make_unique<MixerStrip>(p, i);
        strips[static_cast<size_t>(i)]->onSelect = [this](int bus) { selectChannel(bus); };
        addAndMakeVisible(*strips[static_cast<size_t>(i)]);
    }
    for (auto* control : { &size, &damping, &width })
    {
        slider(*control, juce::Slider::LinearHorizontal, 50); control->setTextBoxStyle(juce::Slider::TextBoxRight, false, 43, 20);
        control->textFromValueFunction = [](double value) { return juce::String(juce::roundToInt(value*100)) + "%"; };
        control->valueFromTextFunction = [](const juce::String& value) { return value.getDoubleValue()*.01; };
        addAndMakeVisible(*control);
    }
    size.setName("Reverb size"); damping.setName("Reverb damping"); width.setName("Reverb width");
    sizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "roomSize", size);
    dampingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "damping", damping);
    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "width", width);
    for (auto* control : { &size, &damping, &width })
    {
        control->textFromValueFunction = [](double v) { return juce::String(juce::roundToInt(v*100)) + "%"; };
        control->valueFromTextFunction = [](const juce::String& v) { return v.getDoubleValue()*.01; };
        control->updateText();
    }
    setResizable(true, true); setResizeLimits(1120, 740, 1800, 1160); setSize(1280, 820); startTimerHz(30);
}
RaBubanjEditor::~RaBubanjEditor() { stopTimer(); setLookAndFeel(nullptr); }
void RaBubanjEditor::selectChannel(int bus)
{
    if (bus < 9) { processor.selectChannel(bus); channelEq.setBus(bus); }
    for (auto& strip : strips) strip->refresh();
}
void RaBubanjEditor::resized()
{
    auto area = getLocalBounds().reduced(22, 16); area.removeFromTop(61);
    eqArea = area.removeFromTop(juce::jlimit(270, 335, getHeight()*36/100));
    auto eqs = eqArea; const int globalWidth = juce::jmax(330, eqs.getWidth()*33/100);
    globalEq.setBounds(eqs.removeFromRight(globalWidth)); eqs.removeFromRight(10); channelEq.setBounds(eqs);
    area.removeFromTop(32); footerArea = area.removeFromBottom(28); mixerArea = area;
    const int stripWidth = juce::jlimit(78, 104, (mixerArea.getWidth()-210)/10);
    auto mixer = mixerArea;
    for (int i = 0; i < 10; ++i)
    {
        if (i == 8) mixer.removeFromLeft(8);
        strips[static_cast<size_t>(i)]->setBounds(mixer.removeFromLeft(stripWidth).reduced(2, 0));
    }
    mixer.removeFromLeft(10); reverbArea = mixer;
    auto controls = reverbArea.reduced(12, 10); controls.removeFromTop(56);
    controls.removeFromTop(16); size.setBounds(controls.removeFromTop(25)); controls.removeFromTop(29);
    damping.setBounds(controls.removeFromTop(25)); controls.removeFromTop(29); width.setBounds(controls.removeFromTop(25));
}
void RaBubanjEditor::paint(juce::Graphics& g)
{
    g.fillAll(background);
    g.setColour(juce::Colour(0xff30363e)); g.fillRect(0, 0, getWidth(), 58);
    text(g, "RaBubanj", { 23, 9, 220, 35 }, 28, ink);
    text(g, "V O Y A G E R S Q U A R E", { 223, 17, 260, 21 }, 10, cyan);
    text(g, "STUDIO MIXER   /   8 DRUMS + REVERB   /   v0.2", { getWidth()-460, 16, 433, 24 }, 11, muted, juce::Justification::centredRight);
    text(g, "MIXER  /  SELECT A CHANNEL TO EDIT ITS EQ", { mixerArea.getX(), mixerArea.getY()-24, 500, 18 }, 10, muted);
    g.setColour(panel); g.fillRoundedRectangle(reverbArea.toFloat(), 4);
    text(g, "REVERB", reverbArea.reduced(12).removeFromTop(24), 13, gold);
    text(g, "SHARED STEREO BUS", { reverbArea.getX()+12, reverbArea.getY()+38, reverbArea.getWidth()-24, 20 }, 9, muted);
    text(g, "SIZE", { size.getX(), size.getY()-17, size.getWidth(), 15 }, 9);
    text(g, "DAMPING", { damping.getX(), damping.getY()-17, damping.getWidth(), 15 }, 9);
    text(g, "WIDTH", { width.getX(), width.getY()-17, width.getWidth(), 15 }, 9);
    const auto x = reverbArea.getX()+12, w = reverbArea.getWidth()-24, y = width.getBottom()+25;
    text(g, "POST-FADER SENDS", { x, y, w, 18 }, 9, cyan);
    text(g, "Drum EQ > level / pan", { x, y+24, w, 18 }, 10);
    text(g, "Send > reverb > return EQ", { x, y+43, w, 18 }, 10);
    text(g, "Master EQ > stereo out", { x, y+62, w, 18 }, 10);
    text(g, "DRAG EQ POINTS: FREQ / GAIN     WHEEL: Q     DOUBLE-CLICK: RESET GAIN", footerArea, 9, muted);
    text(g, "GM MIDI  /  GENERATED KIT", footerArea, 9, muted, juce::Justification::centredRight);
}
void RaBubanjEditor::timerCallback()
{
    if (processor.getSelectedChannel() != channelEq.getBus()) channelEq.setBus(processor.getSelectedChannel());
    channelEq.refresh(); globalEq.refresh(); for (auto& strip : strips) strip->refresh();
}
