#include "PluginEditor.h"
using namespace rabubanj::ui;

MixerLookAndFeel::MixerLookAndFeel()
{
    setColour(juce::TextButton::buttonColourId, juce::Colour(0xff454841));
    setColour(juce::TextButton::buttonOnColourId, cyan);
    setColour(juce::TextButton::textColourOffId, ink); setColour(juce::TextButton::textColourOnId, ink);
    setColour(juce::Slider::backgroundColourId, dark); setColour(juce::Slider::thumbColourId, cyan);
    setColour(juce::PopupMenu::backgroundColourId, menuFace); setColour(juce::PopupMenu::textColourId, axis);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff795e1a));
    setColour(juce::PopupMenu::highlightedTextColourId, ink);
    setColour(juce::TextEditor::backgroundColourId, dark); setColour(juce::TextEditor::textColourId, ink);
    setColour(juce::TooltipWindow::backgroundColourId, menuFace); setColour(juce::TooltipWindow::textColourId, juce::Colour(0xffeee2be));
}
void MixerLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& colour, bool hover, bool down)
{
    const bool previewLit = button.getName().startsWith("Preview ") && (down || static_cast<bool>(button.getProperties()["previewLit"]));
    auto base = previewLit ? juce::Colour(0xffffd454) : juce::Colour(0xff444841).brighter(hover ? .13f : 0.0f);
    if (!previewLit && button.getToggleState()) base = base.interpolatedWith(colour, .35f);
    const auto bounds = button.getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff302c20)); g.fillRoundedRectangle(bounds, 2);
    g.setColour(juce::Colours::black.withAlpha(.85f)); g.fillRoundedRectangle(bounds.reduced(1).translated(0, 1), 1.5f);
    auto face = bounds.reduced(2).translated(0, down ? 1.0f : 0.0f); if (down) face.removeFromBottom(1);
    g.setGradientFill(juce::ColourGradient(base.brighter(.22f), face.getTopLeft(), base.darker(.25f), face.getBottomRight(), false)); g.fillRoundedRectangle(face, 1.5f);
    g.setColour(juce::Colours::white.withAlpha(down ? .1f : .4f)); g.drawLine(face.getX()+1, face.getY()+.5f, face.getRight()-1, face.getY()+.5f, .8f);
    g.setColour(juce::Colours::black.withAlpha(.8f)); g.drawLine(face.getX()+1, face.getBottom(), face.getRight()-1, face.getBottom(), 1.4f);
    if (button.getProperties().contains("channelLevel"))
    {
        const auto level = static_cast<float>(button.getProperties()["channelLevel"]);
        const auto amount = juce::jlimit(0.0f, 1.0f, (juce::Decibels::gainToDecibels(level, -60.0f)+60)/60);
        auto bar = bounds.reduced(3).withHeight(3).withY(bounds.getBottom()-5);
        g.setColour(dark); g.fillRect(bar); bar.setWidth(bar.getWidth()*amount);
        g.setColour(level >= .99f ? juce::Colour(0xffed9b59) : juce::Colour(0xff97d38b)); g.fillRect(bar);
    }
    if (button.getToggleState())
    {
        g.setColour(colour.brighter(.2f)); g.fillRoundedRectangle(4, 3, juce::jmax(2.0f, bounds.getWidth()-8), 2, .5f);
    }
}
void MixerLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float position, float start, float end, juce::Slider& s)
{
    const auto radius = juce::jmin(static_cast<float>(w), static_cast<float>(h))*.5f-8;
    if (radius < 3) return;
    const auto cx = static_cast<float>(x)+static_cast<float>(w)*.5f, cy = static_cast<float>(y)+static_cast<float>(h)*.5f;
    const auto angle = start+position*(end-start);
    const auto name = s.getName();
    auto cap = name.containsIgnoreCase("drive") ? juce::Colour(0xff8eb790)
        : name.containsIgnoreCase("compression") || name.containsIgnoreCase("width") ? juce::Colour(0xffd9857b)
        : name.containsIgnoreCase("damping") ? juce::Colour(0xffd2cdb7) : juce::Colour(0xff8abbd0);
    if (!s.isEnabled()) cap = juce::Colour(0xff91958c);
    for (int tick = 0; tick <= 10; ++tick)
    {
        const auto a = start+static_cast<float>(tick)*(end-start)/10;
        g.setColour(ink.withAlpha(s.isEnabled() ? .9f : .4f));
        g.drawLine(cx+std::sin(a)*(radius+4), cy-std::cos(a)*(radius+4), cx+std::sin(a)*(radius+6), cy-std::cos(a)*(radius+6), .8f);
    }
    g.setColour(juce::Colours::black.withAlpha(.5f)); g.fillEllipse(cx-radius-2, cy-radius+2, radius*2+6, radius*2+6);
    g.setColour(juce::Colour(0xff32362f)); g.fillEllipse(cx-radius-1, cy-radius, radius*2+2, radius*2+2);
    const juce::Rectangle<float> body(cx-radius, cy-radius*.8f, radius*2, radius*1.8f);
    juce::ColourGradient plastic(juce::Colour(0xff969c96), cx-radius, cy, juce::Colour(0xff747d77), cx+radius, cy, false);
    plastic.addColour(.35, juce::Colour(0xffeceee3)); plastic.addColour(.62, juce::Colour(0xffcdd1c8));
    g.setGradientFill(plastic); g.fillEllipse(body);
    {
        juce::Graphics::ScopedSaveState saved(g); juce::Path clip; clip.addEllipse(body); g.reduceClipRegion(clip);
        for (int rib = -10; rib <= 10; ++rib)
        {
            const auto rx = cx+static_cast<float>(rib)*radius/10;
            g.setColour(juce::Colour(0xff626c65).withAlpha(.65f)); g.drawLine(rx, cy, rx, cy+radius, .7f);
            g.setColour(juce::Colours::white.withAlpha(.5f)); g.drawLine(rx+.8f, cy, rx+.8f, cy+radius, .5f);
        }
    }
    const juce::Rectangle<float> top(cx-radius*.86f, cy-radius*.96f, radius*1.72f, radius*1.55f);
    g.setColour(juce::Colour(0xffedf0e6)); g.fillEllipse(top.expanded(1));
    g.setGradientFill(juce::ColourGradient(cap.brighter(.25f), top.getTopLeft(), cap.darker(.22f), top.getBottomRight(), false)); g.fillEllipse(top);
    g.setColour(juce::Colour(0xff6e7770)); g.drawEllipse(top, .6f);
    const auto topCy = top.getCentreY();
    g.setColour(juce::Colour(0xff283630));
    g.drawLine(cx+std::sin(angle)*radius*.16f, topCy-std::cos(angle)*radius*.15f,
        cx+std::sin(angle)*radius*.69f, topCy-std::cos(angle)*radius*.61f, 2.4f);
    g.setColour(juce::Colours::white.withAlpha(.35f)); g.drawEllipse(top.reduced(1.1f), .6f);
}
void MixerLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float position, float minimum, float maximum, juce::Slider::SliderStyle style, juce::Slider& s)
{
    if (style != juce::Slider::LinearVertical && style != juce::Slider::LinearHorizontal)
    { LookAndFeel_V4::drawLinearSlider(g, x, y, w, h, position, minimum, maximum, style, s); return; }
    if (style == juce::Slider::LinearVertical)
    {
        const float cx = static_cast<float>(x)+static_cast<float>(w)*.5f;
        g.setColour(juce::Colour(0xffddc379)); g.drawLine(cx+3, static_cast<float>(y), cx+3, static_cast<float>(y+h), .8f);
        g.setColour(juce::Colour(0xff28271e)); g.fillRoundedRectangle(cx-2.5f, static_cast<float>(y), 5, static_cast<float>(h), 2);
        const bool sendFader = s.getName().containsIgnoreCase("send");
        const auto width = sendFader ? 15.0f : 21.0f, height = sendFader ? 17.0f : 24.0f;
        juce::Rectangle<float> thumb(cx-width*.5f, position-height*.5f, width, height);
        g.setColour(juce::Colours::black.withAlpha(.7f)); g.fillRoundedRectangle(thumb.translated(2, 3).expanded(1), 2);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff737a73), thumb.getTopLeft(), juce::Colour(0xff151c18), thumb.getBottomRight(), false)); g.fillRoundedRectangle(thumb, 2);
        g.setColour(juce::Colour(0xffb5bdb2)); g.drawLine(thumb.getX()+1, thumb.getY()+1, thumb.getRight()-1, thumb.getY()+1, .8f);
        for (int rib = -2; rib <= 2; ++rib)
        { g.setColour(juce::Colour(0xff101812)); g.drawHorizontalLine(juce::roundToInt(position)+rib*3, thumb.getX()+2, thumb.getRight()-2); }
        g.setColour(juce::Colour(0xffe7e9db)); g.fillRect(thumb.getX()+2, position-1, width-4, 2.0f);
    }
    else
    {
        const auto cy = static_cast<float>(y)+static_cast<float>(h)*.5f;
        g.setColour(dark); g.fillRoundedRectangle(static_cast<float>(x), cy-2, static_cast<float>(w), 4, 2);
        g.setColour(juce::Colour(0xff373d35)); g.fillRoundedRectangle(position-4, cy-6, 8, 12, 1);
        g.setColour(juce::Colour(0xffedeede)); g.fillRect(position-1, cy-5, 2.0f, 9.0f);
    }
}

MixerStrip::MixerStrip(RaBubanjProcessor& p, int busToUse) : processor(p), bus(busToUse),
    channelName(bus < 8 ? rabubanj::names[static_cast<size_t>(bus)] : bus == 8 ? "Reverb" : "Master")
{
    setName(channelName + " channel"); addMouseListener(this, true);
    select.setButtonText(channelName); select.setName("Select " + channelName);
    select.setColour(juce::TextButton::buttonOnColourId, bus >= 8 ? gold : cyan);
    select.onClick = [this] { if (onSelect) onSelect(bus); };
    select.getProperties().set("channelLevel", 0.0f);
    select.setTooltip("Select "+channelName+" EQ; the bar shows its live level");
    for (auto* b : { &select, &mute }) addAndMakeVisible(*b);
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
        send.setColour(juce::Slider::trackColourId, gold); send.setDoubleClickReturnValue(true, 0);
        send.setTooltip("Post-fader send to the shared Reverb bus"); addAndMakeVisible(send);
        gainAttachment = std::make_unique<SliderAttachment>(p.parameters, "gain" + id, gain);
        panAttachment = std::make_unique<SliderAttachment>(p.parameters, "pan" + id, pan);
        sendAttachment = std::make_unique<SliderAttachment>(p.parameters, "send" + id, send);
        muteAttachment = std::make_unique<ButtonAttachment>(p.parameters, "mute" + id, mute);
        preview.setName("Preview "+channelName); preview.setTriggeredOnMouseDown(true);
        preview.setColour(juce::TextButton::textColourOffId, ink);
        preview.onClick = [this]
        {
            processor.audition(bus); previewUntil = juce::Time::getMillisecondCounter()+140;
            preview.getProperties().set("previewLit", true); preview.setColour(juce::TextButton::textColourOffId, dark); preview.repaint();
        };
        preview.setTooltip("Play the sample currently loaded on "+channelName);
        clear.setName(channelName+" clear sample"); clear.setTooltip("Remove the imported sample");
        clear.onClick = [this] { processor.clearSample(bus); refresh(); };
        for (auto* b : { &preview, &clear }) { b->setWantsKeyboardFocus(false); addAndMakeVisible(*b); }
        sampleName.setFont(juce::FontOptions(10.0f)); sampleName.setJustificationType(juce::Justification::centred);
        sampleName.setColour(juce::Label::textColourId, ink); sampleName.setBorderSize(juce::BorderSize<int>()); addAndMakeVisible(sampleName);
        sampleName.setName(channelName+" sample source");
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
    for (auto* b : { &select, &mute, &solo }) b->setWantsKeyboardFocus(false);
    refresh();
}
void MixerStrip::mouseDown(const juce::MouseEvent&) { if (onSelect) onSelect(bus); }
void MixerStrip::resized()
{
    auto area = getLocalBounds().reduced(6, 8);
    select.setBounds(area.removeFromTop(25)); area.removeFromTop(5);
    auto sourceRow = area.removeFromTop(19); clear.setBounds(sourceRow.removeFromRight(14)); sampleName.setBounds(sourceRow); area.removeFromTop(5);
    auto bottom = area.removeFromBottom(56);
    auto buttons = bottom.removeFromTop(21);
    mute.setBounds(buttons.removeFromLeft(buttons.getWidth()/2).reduced(1, 0)); solo.setBounds(buttons.reduced(1, 0));
    bottom.removeFromTop(7); preview.setBounds(bottom.removeFromTop(21));
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
    raisedPanel(g, getLocalBounds().toFloat(), selected ? panel.brighter(.1f) : panel, .9f);
    if (dragging) { g.setColour(cyan.withAlpha(.18f)); g.fillRect(getLocalBounds()); g.setColour(cyan); g.drawRect(getLocalBounds().reduced(2), 2); }
    g.setColour(selected ? colour : colour.darker(.5f)); g.fillRect(0, 0, getWidth(), 3);
    g.setColour(juce::Colour(0xff6d5412)); g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(.5f), 3, .7f);
    text(g, bus < 8 ? "PAN" : bus == 8 ? "WET RETURN" : "STEREO OUT", { 8, 63, getWidth()-16, 13 }, 8, muted, juce::Justification::centred);
    text(g, "dB", { gain.getX(), faderArea.getY()-15, gain.getWidth(), 13 }, 9, muted, juce::Justification::centred);
    if (bus < 8) text(g, "SEND", { send.getX(), faderArea.getY()-15, send.getWidth(), 13 }, 8, ink, juce::Justification::centred);
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
    select.getProperties().set("channelLevel", processor.getPeak(bus)); select.repaint();
    select.setToggleState(bus == 9 || bus == processor.getSelectedChannel(), juce::dontSendNotification);
    if (bus < 8)
    {
        const auto info = processor.getSampleInfo(bus);
        const bool lit = previewUntil != 0 && static_cast<int32_t>(previewUntil-juce::Time::getMillisecondCounter()) > 0;
        preview.getProperties().set("previewLit", lit);
        preview.setColour(juce::TextButton::textColourOffId, lit || preview.isDown() ? dark : ink);
        const bool loading = info.status == SampleLoader::Status::Loading;
        const bool problem = info.status == SampleLoader::Status::Error || info.status == SampleLoader::Status::Missing;
        sampleName.setText(dragging ? "DROP SAMPLE" : loading ? "Loading..." : problem ? "Load error" : info.active ? info.name : "Drop sample", juce::dontSendNotification);
        sampleName.setColour(juce::Label::textColourId, problem ? juce::Colour(0xffeea18c) : muted);
        sampleName.setTooltip(info.message.isNotEmpty() ? info.message : "Drag a mono/stereo audio file here from your DAW browser");
        clear.setVisible(info.active || loading || problem);
    }
    repaint();
}

bool MixerStrip::isInterestedInFileDrag(const juce::StringArray& paths) { return bus < 8 && SampleLoader::dropFile(paths) != juce::File(); }
void MixerStrip::fileDragEnter(const juce::StringArray& paths, int, int) { dragging = isInterestedInFileDrag(paths); refresh(); }
void MixerStrip::fileDragExit(const juce::StringArray&) { dragging = false; refresh(); }
void MixerStrip::filesDropped(const juce::StringArray& paths, int, int)
{
    dragging = false;
    if (isInterestedInFileDrag(paths)) { if (onSelect) onSelect(bus); processor.importSample(bus, SampleLoader::dropFile(paths)); }
    refresh();
}
static juce::StringArray samplePaths(const juce::String& text)
{
    auto paths = juce::StringArray::fromLines(text); paths.removeEmptyStrings();
    for (int i = paths.size()-1; i >= 0; --i) if (paths[i].trimStart().startsWith("#")) paths.remove(i);
    return paths;
}
bool MixerStrip::isInterestedInTextDrag(const juce::String& text) { return isInterestedInFileDrag(samplePaths(text)); }
void MixerStrip::textDragEnter(const juce::String& text, int x, int y) { fileDragEnter(samplePaths(text), x, y); }
void MixerStrip::textDragExit(const juce::String&) { fileDragExit({}); }
void MixerStrip::textDropped(const juce::String& text, int x, int y) { filesDropped(samplePaths(text), x, y); }

RaBubanjEditor::RaBubanjEditor(RaBubanjProcessor& p)
    : AudioProcessorEditor(&p), processor(p), channelEq(p, p.getSelectedChannel()), globalEq(p, rabubanj::masterBus, true)
{
    processor.setAnalyserActive(true);
    setLookAndFeel(&look); setWantsKeyboardFocus(true); addAndMakeVisible(channelEq); addAndMakeVisible(globalEq);
    keyboardMode.setName("Computer keyboard mode"); midiMode.setName("MIDI note mapping");
    keyboardMode.setTooltip("DAW: let the host handle keyboard MIDI. PLUGIN: audition A W S E D F T G while this window has focus (outside recording).");
    midiMode.setTooltip("Chromatic: C1 to G1 (MIDI 36-43). GM: use the older General MIDI drum mapping.");
    keyboardMode.onClick = [this] { processor.setLocalKeys(!processor.localKeysEnabled()); heldKeys.fill(false); if (isShowing()) grabKeyboardFocus(); timerCallback(); };
    midiMode.onClick = [this]
    {
        auto* parameter = processor.parameters.getParameter("midiMap"); parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->getValue() < .5f ? 1.0f : 0.0f); parameter->endChangeGesture(); timerCallback();
    };
    for (auto* b : { &keyboardMode, &midiMode }) { b->setWantsKeyboardFocus(false); addAndMakeVisible(*b); }
    for (int i = 0; i < rabubanj::eqBusCount; ++i)
    {
        strips[static_cast<size_t>(i)] = std::make_unique<MixerStrip>(p, i);
        strips[static_cast<size_t>(i)]->onSelect = [this](int bus) { selectChannel(bus); };
        addAndMakeVisible(*strips[static_cast<size_t>(i)]);
    }
    for (auto* control : { &size, &damping, &width })
    {
        slider(*control, juce::Slider::RotaryHorizontalVerticalDrag, 50); control->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 19);
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
    for (auto* control : { &pitch, &drive, &compression })
    {
        slider(*control, juce::Slider::RotaryHorizontalVerticalDrag, 50);
        control->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 53, 19); control->setDoubleClickReturnValue(true, 0); addAndMakeVisible(*control);
    }
    pitch.setName("Channel pitch"); drive.setName("Channel drive"); compression.setName("Channel compression");
    pitch.setTooltip("Pitch of the selected drum: -24 to +24 semitones. Changes playback speed and duration. Double-click resets.");
    drive.setTooltip("Saturation of the selected drum, with antialiasing. Double-click resets.");
    compression.setTooltip("Compression strength: -18 dBFS threshold, 3:1 ratio, 6 dB soft knee, 10 ms attack, 150-350 ms opto-style release. Stereo linked; no makeup gain.");
    bindChannelControls();
    setResizable(true, true); setResizeLimits(1120, 740, 1800, 1160); setSize(1280, 820); timerCallback(); startTimerHz(30);
}
RaBubanjEditor::~RaBubanjEditor() { processor.setAnalyserActive(false); stopTimer(); setLookAndFeel(nullptr); }
void RaBubanjEditor::selectChannel(int bus)
{
    if (bus < 9) { processor.selectChannel(bus); channelEq.setBus(bus); }
    if (processor.localKeysEnabled() && isShowing() && dynamic_cast<juce::TextEditor*>(juce::Component::getCurrentlyFocusedComponent()) == nullptr) grabKeyboardFocus();
    bindChannelControls(); for (auto& strip : strips) strip->refresh(); repaint();
}
void RaBubanjEditor::bindChannelControls()
{
    const auto bus = processor.getSelectedChannel(); if (bus == colourBus) return;
    pitchAttachment.reset(); driveAttachment.reset(); compAttachment.reset(); colourBus = bus;
    const bool drum = bus < 8;
    for (auto* control : { &pitch, &drive, &compression }) control->setEnabled(drum);
    if (!drum) return;
    const auto id = juce::String(bus);
    pitchAttachment = std::make_unique<SliderAttachment>(processor.parameters, "pitch"+id, pitch);
    driveAttachment = std::make_unique<SliderAttachment>(processor.parameters, "drive"+id, drive);
    compAttachment = std::make_unique<SliderAttachment>(processor.parameters, "comp"+id, compression);
    pitch.textFromValueFunction = [](double v) { return juce::String(v, 1)+" st"; };
    pitch.valueFromTextFunction = [](const juce::String& v) { return v.getDoubleValue(); };
    for (auto* control : { &drive, &compression })
    {
        control->textFromValueFunction = [](double v) { return juce::String(juce::roundToInt(v*100))+"%"; };
        control->valueFromTextFunction = [](const juce::String& v) { return v.getDoubleValue()*.01; };
    }
    pitch.updateText(); drive.updateText(); compression.updateText();
}
void RaBubanjEditor::resized()
{
    midiMode.setBounds(getWidth()-285, 23, 135, 24); keyboardMode.setBounds(getWidth()-141, 23, 119, 24);
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
    auto halves = reverbArea;
    reverbControlsArea = halves.removeFromLeft(halves.getWidth()/2).withTrimmedRight(3);
    reservedArea = halves.withTrimmedLeft(3);
    auto controls = reverbControlsArea.reduced(8, 8); controls.removeFromTop(26);
    const auto rowHeight = controls.getHeight()/3;
    for (auto* control : { &size, &damping, &width })
    {
        auto row = controls.removeFromTop(rowHeight); row.removeFromTop(14); row.removeFromBottom(4); control->setBounds(row);
    }
    controls = reservedArea.reduced(8, 8); controls.removeFromTop(26);
    for (auto* control : { &pitch, &drive, &compression })
    {
        auto row = controls.removeFromTop(rowHeight); row.removeFromTop(14); row.removeFromBottom(4); control->setBounds(row);
    }
}
void RaBubanjEditor::paint(juce::Graphics& g)
{
    g.fillAll(background);
    raisedPanel(g, getLocalBounds().toFloat(), background.brighter(.18f), 1.0f);
    raisedPanel(g, { 4, 3, static_cast<float>(getWidth()-8), 55 }, panel, 1.0f);
    text(g, "RaBubanj", { 23, 9, 220, 35 }, 28, ink);
    screw(g, 12, 28); screw(g, static_cast<float>(getWidth()-13), 28);
    text(g, "MIXER  /  SELECT A CHANNEL TO EDIT ITS EQ", { mixerArea.getX(), mixerArea.getY()-24, 500, 18 }, 10, muted);
    raisedPanel(g, reverbControlsArea.toFloat(), panel, 1.0f);
    raisedPanel(g, reservedArea.toFloat(), panel.darker(.1f), 1.0f);
    text(g, "REVERB", reverbControlsArea.reduced(8).removeFromTop(20), 11, ink, juce::Justification::centred);
    text(g, "SIZE", { size.getX(), size.getY()-14, size.getWidth(), 13 }, 9, muted, juce::Justification::centred);
    text(g, "DAMPING", { damping.getX(), damping.getY()-14, damping.getWidth(), 13 }, 9, muted, juce::Justification::centred);
    const auto colourTitle = colourBus >= 0 && colourBus < 8 ? juce::String(rabubanj::names[static_cast<size_t>(colourBus)]).toUpperCase() : "DRUM FX";
    text(g, colourTitle, reservedArea.reduced(5).removeFromTop(20), 10, ink, juce::Justification::centred);
    for (const auto& item : { std::pair<juce::Slider*, const char*>(&pitch, "PITCH"), { &drive, "DRIVE" }, { &compression, "COMP" } })
        text(g, item.second, { item.first->getX(), item.first->getY()-14, item.first->getWidth(), 13 }, 9, muted, juce::Justification::centred);
    text(g, "WIDTH", { width.getX(), width.getY()-14, width.getWidth(), 13 }, 9, muted, juce::Justification::centred);
    text(g, "DRAG EQ: FREQ / GAIN    WHEEL: Q    RIGHT-CLICK: SHAPE / GAIN", footerArea, 9, muted);
    text(g, "C1-G1 MIDI  /  SAMPLE PLAYER", footerArea, 9, muted, juce::Justification::centredRight);
}
void RaBubanjEditor::timerCallback()
{
    keyboardMode.setButtonText(processor.localKeysEnabled() ? "KEYS: PLUGIN" : "KEYS: DAW");
    midiMode.setButtonText(processor.parameters.getRawParameterValue("midiMap")->load() < .5f ? "MIDI: C1-G1" : "MIDI: GM");
    if (processor.getSelectedChannel() != channelEq.getBus()) channelEq.setBus(processor.getSelectedChannel());
    bindChannelControls(); channelEq.refresh(); globalEq.refresh(); for (auto& strip : strips) strip->refresh();
}

static constexpr std::array<int, 8> auditionKeys { 'A', 'W', 'S', 'E', 'D', 'F', 'T', 'G' };
bool RaBubanjEditor::keyPressed(const juce::KeyPress& key)
{
    if (!processor.localKeysEnabled() || processor.isHostRecording() || key.getModifiers().isCommandDown()
        || key.getModifiers().isCtrlDown() || key.getModifiers().isAltDown()
        || dynamic_cast<juce::TextEditor*>(juce::Component::getCurrentlyFocusedComponent()) != nullptr) return false;
    const auto code = juce::CharacterFunctions::toUpperCase(static_cast<juce::juce_wchar>(key.getKeyCode()));
    for (size_t i = 0; i < auditionKeys.size(); ++i) if (code == static_cast<juce::juce_wchar>(auditionKeys[i]))
    {
        if (!heldKeys[i]) { heldKeys[i] = true; processor.audition(static_cast<int>(i)); }
        return true;
    }
    return false;
}
bool RaBubanjEditor::keyStateChanged(bool)
{
    for (size_t i = 0; i < heldKeys.size(); ++i) if (!juce::KeyPress::isKeyCurrentlyDown(auditionKeys[i])) heldKeys[i] = false;
    return false;
}
void RaBubanjEditor::focusLost(FocusChangeType) { heldKeys.fill(false); }
