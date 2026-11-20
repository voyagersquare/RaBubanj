#include "PluginEditor.h"
using namespace rabubanj::ui;

MixerLookAndFeel::MixerLookAndFeel()
{
    setColour(juce::TextButton::buttonColourId, juce::Colour(0xff454841));
    setColour(juce::TextButton::buttonOnColourId, cyan);
    setColour(juce::TextButton::textColourOffId, ink); setColour(juce::TextButton::textColourOnId, ink);
    setColour(juce::Slider::backgroundColourId, dark); setColour(juce::Slider::thumbColourId, cyan);
    setColour(juce::ComboBox::backgroundColourId, menuFace); setColour(juce::ComboBox::textColourId, ink);
    setColour(juce::ComboBox::outlineColourId, axis); setColour(juce::ComboBox::arrowColourId, gold);
    setColour(juce::PopupMenu::backgroundColourId, menuFace); setColour(juce::PopupMenu::textColourId, axis);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff795e1a));
    setColour(juce::PopupMenu::highlightedTextColourId, ink);
    setColour(juce::TextEditor::backgroundColourId, dark); setColour(juce::TextEditor::textColourId, ink);
    setColour(juce::TooltipWindow::backgroundColourId, menuFace); setColour(juce::TooltipWindow::textColourId, juce::Colour(0xffeee2be));
}
juce::Button* MixerLookAndFeel::createDocumentWindowButton(int type)
{
    auto* button = new juce::TextButton(type == juce::DocumentWindow::closeButton ? "x" : type == juce::DocumentWindow::minimiseButton ? "-" : "+");
    button->setName("Window control"); button->setWantsKeyboardFocus(false); return button;
}
void MixerLookAndFeel::drawDocumentWindowTitleBar(juce::DocumentWindow& window, juce::Graphics& g, int width, int height, int titleX, int titleWidth, const juce::Image*, bool)
{
    g.fillAll(menuFace); g.setColour(gold.darker(.35f)); g.drawRect(0, 0, width, height, 1);
    text(g, window.getName(), { titleX, 0, titleWidth, height }, 15, ink, juce::Justification::centred, true);
}
void MixerLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& colour, bool hover, bool down)
{
    const bool previewLit = button.getName().startsWith("Preview ") && (down || static_cast<bool>(button.getProperties()["previewLit"]));
    auto base = button.getName().startsWith("Close ") ? juce::Colour(hover ? 0xffec5951 : 0xffbd3832) : previewLit ? juce::Colour(0xffffd454) : juce::Colour(0xff8b97a5).brighter(hover ? .16f : 0.0f);
    if (!previewLit && button.getToggleState()) base = base.interpolatedWith(colour, .35f);
    const auto bounds = button.getLocalBounds().toFloat();
    const auto lift = down ? 2.0f : 5.0f;
    auto face = bounds.reduced(2).withTrimmedBottom(lift);
    if (down) face = face.translated(0, 2);
    metalSurface(g, bounds.reduced(1), steelDark);
    g.setColour(juce::Colours::black.withAlpha(.55f)); g.fillRoundedRectangle(bounds.reduced(1).withTrimmedTop(4), 2);
    metalSurface(g, face, base);
    g.setColour(juce::Colour(0xfff1f7ff).withAlpha(.8f)); g.drawLine(face.getX()+1, face.getY()+1, face.getRight()-1, face.getY()+1, 1.2f);
    g.setColour(juce::Colour(0xff151e2b)); g.drawRoundedRectangle(bounds.reduced(1), 2, 1);
    g.setColour(juce::Colours::black.withAlpha(.75f)); g.drawLine(face.getX(), face.getBottom(), face.getRight(), face.getBottom(), 1.5f);
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
void MixerLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool down)
{
    const auto colour = button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId : juce::TextButton::textColourOffId);
    text(g, button.getButtonText(), button.getLocalBounds().withTrimmedBottom(down ? 1 : 4).reduced(3, 0),
        getTextButtonFont(button, button.getHeight()).getHeight(), colour, juce::Justification::centred, true);
}
void MixerLookAndFeel::drawComboBox(juce::Graphics& g, int w, int h, bool down, int, int, int, int, juce::ComboBox&)
{
    raisedMetal(g, { 0, 0, static_cast<float>(w), static_cast<float>(h) }, steel, 4);
    g.setColour(juce::Colours::black.withAlpha(down ? .65f : .45f)); g.fillRoundedRectangle(3, 2, static_cast<float>(w-6), static_cast<float>(h-7), 2);
    juce::Path arrow; const auto cx = static_cast<float>(w-15), cy = static_cast<float>(h)*.43f;
    arrow.startNewSubPath(cx-4, cy-2); arrow.lineTo(cx, cy+2); arrow.lineTo(cx+4, cy-2);
    g.setColour(gold); g.strokePath(arrow, juce::PathStrokeType(1.8f));
}
void MixerLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float position, float start, float end, juce::Slider& s)
{
    // Keep the approved reference silhouette and scale the complete rotary artwork by 70%.
    // Slider bounds and mouse/automation behaviour remain independent of the rendered size.
    juce::Graphics::ScopedSaveState savedKnob(g);
    g.addTransform(juce::AffineTransform::scale(.7f, .7f, static_cast<float>(x)+static_cast<float>(w)*.5f, static_cast<float>(y)+static_cast<float>(h)*.5f));
    const auto diameter = juce::jmin(static_cast<float>(w)-14, (static_cast<float>(h)-9)/1.7f);
    if (diameter < 6) return;
    const auto radius = diameter*.5f, rise = diameter*.98f, capHeight = diameter*.72f;
    const auto cx = static_cast<float>(x)+static_cast<float>(w)*.5f;
    const auto topY = static_cast<float>(y)+3;
    const juce::Rectangle<float> top(cx-radius, topY, diameter, capHeight), lower = top.translated(0, rise);
    const auto angle = start+position*(end-start), topCy = top.getCentreY();
    const auto name = s.getName();
    const auto colour = !s.isEnabled() ? juce::Colour(0xff72766f)
        : name.containsIgnoreCase("pitch") ? juce::Colour(0xff286b9d)
        : name.containsIgnoreCase("drive") ? juce::Colour(0xffb42c35)
        : name.containsIgnoreCase("compression") ? juce::Colour(0xffa34b9f)
        : name.containsIgnoreCase("damping") ? juce::Colour(0xffbdc0b7)
        : name.containsIgnoreCase("width") ? juce::Colour(0xff78443e) : juce::Colour(0xff258e87);
    g.setColour(juce::Colours::black.withAlpha(.42f)); g.fillEllipse(lower.translated(diameter*.14f, diameter*.14f).expanded(1));
    g.setColour(juce::Colour(0xff101411)); g.fillEllipse(lower.translated(0, diameter*.1f).expanded(.8f));
    juce::Path body; body.addRectangle(top.getX(), topCy, diameter, rise); body.addEllipse(lower);
    juce::ColourGradient wall(juce::Colour(0xff434b46), top.getX(), topCy, juce::Colour(0xff424a44), top.getRight(), topCy, false);
    wall.addColour(.08, juce::Colour(0xffa3aaa1)); wall.addColour(.2, juce::Colour(0xff424d46));
    wall.addColour(.37, juce::Colour(0xff242c27)); wall.addColour(.62, juce::Colour(0xff747e75)); wall.addColour(.8, juce::Colour(0xff505b52));
    g.setGradientFill(wall); g.fillPath(body);
    {
        juce::Graphics::ScopedSaveState saved(g); g.reduceClipRegion(body);
        for (int flute = -2; flute <= 2; ++flute)
        {
            const auto a = static_cast<float>(flute)*.58f;
            const auto fx = cx+std::sin(a)*radius, fy = topCy+std::cos(a)*capHeight*.5f;
            const auto groove = juce::jmax(1.3f, diameter*.075f);
            g.setGradientFill(juce::ColourGradient(juce::Colour(0xff111b15), fx-groove*.5f, fy, juce::Colour(0xff56665a), fx+groove*.5f, fy, false));
            g.fillRect(fx-groove*.5f, fy-2, groove, rise+3);
            g.setColour(juce::Colour(0xffc4cdc1).withAlpha(.55f)); g.drawLine(fx+groove*.5f, fy, fx+groove*.5f, fy+rise, .7f);
        }
    }
    // A subtly scalloped rim corresponds to the flutes; the coloured cap remains a flat disc.
    juce::Path rim;
    for (int point = 0; point <= 120; ++point)
    {
        const auto a = static_cast<float>(point)*juce::MathConstants<float>::twoPi/120;
        const auto ripple = 1-.022f*std::pow(.5f+.5f*std::cos(a*10), 4.0f);
        const juce::Point<float> p(cx+std::sin(a)*radius*ripple, topCy-std::cos(a)*capHeight*.5f*ripple);
        if (point == 0) rim.startNewSubPath(p); else rim.lineTo(p);
    }
    rim.closeSubPath();
    g.setColour(juce::Colour(0xffaeb6aa)); g.fillPath(rim);
    g.setColour(juce::Colour(0xffe3e8dc).withAlpha(.55f)); g.strokePath(rim, juce::PathStrokeType(.7f));
    const auto cap = top.reduced(diameter*.035f, diameter*.026f);
    g.setColour(juce::Colour(0xff18251e)); g.fillEllipse(cap.translated(0, 1));
    g.setGradientFill(juce::ColourGradient(colour.brighter(.045f), cap.getTopLeft(), colour.darker(.12f), cap.getBottomRight(), false)); g.fillEllipse(cap);
    {
        juce::Graphics::ScopedSaveState saved(g); juce::Path clip; clip.addEllipse(cap); g.reduceClipRegion(clip);
        for (int py = juce::roundToInt(cap.getY()); py < juce::roundToInt(cap.getBottom()); ++py)
            for (int px = juce::roundToInt(cap.getX()); px < juce::roundToInt(cap.getRight()); ++px)
            {
                auto grain = static_cast<unsigned>(px)*374761393u+static_cast<unsigned>(py)*668265263u;
                grain = (grain^(grain >> 13u))*1274126177u; grain ^= grain >> 16u;
                if ((grain & 7u) == 0) { g.setColour((grain & 8u) != 0 ? juce::Colours::white.withAlpha(.07f) : juce::Colours::black.withAlpha(.055f)); g.fillRect(px, py, 1, 1); }
            }
    }
    g.setColour(name.containsIgnoreCase("damping") ? juce::Colour(0xff171b17) : juce::Colour(0xfff2f3e8));
    g.drawLine(cx+std::sin(angle)*radius*.08f, topCy-std::cos(angle)*radius*.058f,
        cx+std::sin(angle)*radius*.96f, topCy-std::cos(angle)*radius*.69f, juce::jmax(2.0f, diameter*.095f));
    if (!s.isEnabled()) { g.setColour(juce::Colours::black.withAlpha(.22f)); g.fillPath(body); g.fillEllipse(cap); }
}
namespace
{
void drawControlLegend(juce::Graphics& g, const juce::String& label, juce::Rectangle<int> bounds, float fontSize = 16)
{
    g.setColour(juce::Colour(0xff142019).withAlpha(.86f)); g.fillRoundedRectangle(bounds.toFloat(), 2.5f);
    g.setColour(juce::Colour(0xffe9d6a5).withAlpha(.45f)); g.drawRoundedRectangle(bounds.toFloat().reduced(.5f), 2.5f, .8f);
    text(g, label, bounds.reduced(2, 0), fontSize, ink, juce::Justification::centred, true);
}
void drawSaddleFader(juce::Graphics& g, float cx, float cy, float width, float length)
{
    const auto left = cx-width*.5f, right = cx+width*.5f, top = cy-length*.5f, bottom = cy+length*.5f;
    const auto waist = juce::jmin(3.2f, width*.12f), wing = length*.24f;
    juce::Path cap;
    cap.startNewSubPath(left+1, top); cap.lineTo(right-1, top); cap.quadraticTo(right, top, right, top+2);
    cap.lineTo(right, top+wing); cap.cubicTo(right-waist, cy-length*.15f, right-waist, cy+length*.15f, right, bottom-wing);
    cap.lineTo(right, bottom-2); cap.quadraticTo(right, bottom, right-1, bottom); cap.lineTo(left+1, bottom); cap.quadraticTo(left, bottom, left, bottom-2);
    cap.lineTo(left, bottom-wing); cap.cubicTo(left+waist, cy+length*.15f, left+waist, cy-length*.15f, left, top+wing);
    cap.lineTo(left, top+2); cap.quadraticTo(left, top, left+1, top); cap.closeSubPath();
    const auto extrusion = juce::AffineTransform::translation(0, 5);
    g.setColour(juce::Colours::black.withAlpha(.55f)); g.fillPath(cap, juce::AffineTransform::translation(2, 7));
    g.setColour(juce::Colour(0xff141918)); g.fillPath(cap, extrusion);
    g.setColour(juce::Colour(0xff303936)); g.fillPath(cap);
    {
        juce::Graphics::ScopedSaveState saved(g); g.reduceClipRegion(cap);
        const auto cheek = juce::jmin(4.0f, width*.16f);
        juce::ColourGradient aluminium(juce::Colour(0xffb6b4ad), cx, top, juce::Colour(0xff81817a), cx, bottom, false);
        aluminium.addColour(.12, juce::Colour(0xffe6e3d8)); aluminium.addColour(.3, juce::Colour(0xff888982));
        aluminium.addColour(.5, juce::Colour(0xff5d635b)); aluminium.addColour(.7, juce::Colour(0xffbbc0b6)); aluminium.addColour(.86, juce::Colour(0xffd4d4c8));
        g.setGradientFill(aluminium); g.fillRect(left+cheek, top, width-cheek*2, length);
        g.setColour(juce::Colours::white.withAlpha(.2f)); g.drawVerticalLine(juce::roundToInt(left+cheek)+1, top+1, bottom-1);
        g.setColour(juce::Colours::black.withAlpha(.3f)); g.drawVerticalLine(juce::roundToInt(right-cheek), top+1, bottom-1);
        g.setColour(juce::Colour(0xffeef1e6)); g.fillRect(left+cheek, cy-.8f, width-cheek*2, 1.6f);
        g.setColour(juce::Colour(0xffe6eadf).withAlpha(.65f)); g.drawHorizontalLine(juce::roundToInt(top)+2, left+cheek, right-cheek);
        g.setColour(juce::Colour(0xff26342b)); g.drawHorizontalLine(juce::roundToInt(bottom-wing), left+cheek, right-cheek);
    }
    g.setColour(juce::Colour(0xff111c16)); g.strokePath(cap, juce::PathStrokeType(.9f));
}
}
void MixerLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float position, float minimum, float maximum, juce::Slider::SliderStyle style, juce::Slider& s)
{
    if (style != juce::Slider::LinearVertical && style != juce::Slider::LinearHorizontal)
    { LookAndFeel_V4::drawLinearSlider(g, x, y, w, h, position, minimum, maximum, style, s); return; }
    const bool vertical = style == juce::Slider::LinearVertical;
    const auto cx = static_cast<float>(x)+static_cast<float>(w)*.5f, cy = static_cast<float>(y)+static_cast<float>(h)*.5f;
    const auto track = vertical ? juce::Rectangle<float>(cx-3, static_cast<float>(y), 6, static_cast<float>(h))
        : juce::Rectangle<float>(static_cast<float>(x), cy-2.5f, static_cast<float>(w), 5);
    g.setColour(juce::Colour(0xffe3b47b)); g.fillRoundedRectangle(track.translated(1.5f, 1.5f), 2);
    g.setColour(juce::Colour(0xff171f19)); g.fillRoundedRectangle(track, 2);
    if (vertical)
    {
        const auto send = s.getName().containsIgnoreCase("send");
        drawSaddleFader(g, cx, position, juce::jmin(static_cast<float>(w-2), send ? 21.0f : 29.0f), send ? 26.0f : 32.0f);
    }
    else
    {
        juce::Graphics::ScopedSaveState saved(g); g.addTransform(juce::AffineTransform::rotation(-juce::MathConstants<float>::halfPi, position, cy));
        drawSaddleFader(g, position, cy, juce::jmin(11.0f, static_cast<float>(h)-5), 18);
    }
}

MixerStrip::MixerStrip(RaBubanjProcessor& p, int busToUse) : processor(p), bus(busToUse),
    channelName(bus < 8 ? rabubanj::padNames[static_cast<size_t>(bus)] : bus == 8 ? "Reverb" : "Master")
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
    if (bus < 8) text(g, "PAN", { 8, 61, getWidth()-16, 16 }, 11, ink, juce::Justification::centred, true);
    else
    {
        text(g, bus == 8 ? "WET" : "STEREO", { 5, 56, getWidth()-10, 15 }, 11, ink, juce::Justification::centred, true);
        text(g, bus == 8 ? "RETURN" : "OUT", { 5, 71, getWidth()-10, 15 }, 11, ink, juce::Justification::centred, true);
    }
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
    sequenceButton.setName("Open step sequencer"); sequenceButton.setWantsKeyboardFocus(false); sequenceButton.onClick = [this] { if (sequenceWindow && sequenceWindow->isVisible()) sequenceWindow->closeButtonPressed(); else showSequencer(); }; addAndMakeVisible(sequenceButton);
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
RaBubanjEditor::~RaBubanjEditor() { sequenceWindow.reset(); processor.setAnalyserActive(false); stopTimer(); setLookAndFeel(nullptr); }
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
void RaBubanjEditor::showSequencer()
{
    if (!sequenceWindow) { sequenceWindow = std::make_unique<SequencerWindow>(processor); sequenceWindow->setLookAndFeel(&look); addChildComponent(*sequenceWindow); }
    sequenceWindow->setBounds((getWidth()-juce::jmin(getWidth()-24, 1160))/2, juce::jmax(65, (getHeight()-650)/2), juce::jmin(getWidth()-24, 1160), 650);
    sequenceWindow->setVisible(true); sequenceWindow->toFront(false); if (isShowing()) grabKeyboardFocus();
}
void RaBubanjEditor::resized()
{
    sequenceButton.setBounds(getWidth()-465, 23, 166, 24);
    if (sequenceWindow) sequenceWindow->setBounds((getWidth()-juce::jmin(getWidth()-24, 1160))/2, juce::jmax(65, (getHeight()-650)/2), juce::jmin(getWidth()-24, 1160), 650);
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
    auto controls = reverbControlsArea.reduced(8, 8); controls.removeFromTop(30);
    const auto rowHeight = controls.getHeight()/3;
    for (auto* control : { &size, &damping, &width })
    {
        auto row = controls.removeFromTop(rowHeight); row.removeFromTop(20); row.removeFromBottom(4); control->setBounds(row);
    }
    controls = reservedArea.reduced(8, 8); controls.removeFromTop(30);
    for (auto* control : { &pitch, &drive, &compression })
    {
        auto row = controls.removeFromTop(rowHeight); row.removeFromTop(20); row.removeFromBottom(4); control->setBounds(row);
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
    drawControlLegend(g, "REVERB", reverbControlsArea.reduced(6).removeFromTop(25), 17);
    drawControlLegend(g, "SIZE", { size.getX(), size.getY()-24, size.getWidth(), 22 });
    drawControlLegend(g, "DAMPING", { damping.getX(), damping.getY()-24, damping.getWidth(), 22 });
    const auto colourTitle = colourBus >= 0 && colourBus < 8 ? juce::String(rabubanj::padNames[static_cast<size_t>(colourBus)]).toUpperCase() : "DRUM FX";
    drawControlLegend(g, colourTitle, reservedArea.reduced(6).removeFromTop(25), 17);
    for (const auto& item : { std::pair<juce::Slider*, const char*>(&pitch, "PITCH"), { &drive, "DRIVE" }, { &compression, "COMP" } })
        drawControlLegend(g, item.second, { item.first->getX(), item.first->getY()-24, item.first->getWidth(), 22 });
    drawControlLegend(g, "WIDTH", { width.getX(), width.getY()-24, width.getWidth(), 22 });
    text(g, "DRAG EQ: FREQ / GAIN    WHEEL: Q    RIGHT-CLICK: SHAPE / GAIN", footerArea, 9, muted);
    text(g, "C1-G1 MIDI  /  SAMPLE PLAYER", footerArea, 9, muted, juce::Justification::centredRight);
}
void RaBubanjEditor::timerCallback()
{
    sequenceButton.setToggleState(sequenceWindow && sequenceWindow->isVisible(), juce::dontSendNotification);
    keyboardMode.setButtonText(processor.localKeysEnabled() ? "KEYS: PLUGIN" : "KEYS: DAW");
    midiMode.setButtonText(processor.parameters.getRawParameterValue("midiMap")->load() < .5f ? "MIDI: C1-G1" : "MIDI: GM");
    if (processor.getSelectedChannel() != channelEq.getBus()) channelEq.setBus(processor.getSelectedChannel());
    bindChannelControls(); channelEq.refresh(); globalEq.refresh(); for (auto& strip : strips) strip->refresh();
}

static constexpr std::array<int, 8> auditionKeys { 'A', 'W', 'S', 'E', 'D', 'F', 'T', 'G' };
bool RaBubanjEditor::keyPressed(const juce::KeyPress& key)
{
    if (key.getKeyCode() == juce::KeyPress::spaceKey) return false;
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
