#include "EqPanel.h"
using namespace rabubanj::ui;

EqPanel::EqPanel(RaBubanjProcessor& p, int bus, bool isMaster) : processor(p), masterPanel(isMaster)
{
    setName(isMaster ? "Global EQ" : "Channel EQ");
    for (int b = 0; b < rabubanj::eqBandCount; ++b)
    {
        auto& button = bands[static_cast<size_t>(b)]; button.setButtonText(juce::String(b + 1));
        button.setColour(juce::TextButton::buttonOnColourId, bandColours[static_cast<size_t>(b)]);
        button.setColour(juce::TextButton::textColourOnId, dark);
        button.onClick = [this, b] { selectBand(b); }; addAndMakeVisible(button);
    }
    bypass.setClickingTogglesState(true); bypass.setTooltip("Bypass all six bands of this EQ");
    reset.setTooltip("Reset this EQ only; other channels are unchanged");
    reset.onClick = [this]
    {
        for (int b = 0; b < rabubanj::eqBandCount; ++b)
            for (const auto* field : { "freq", "gain", "q", "type", "on" })
                if (auto* parameterToReset = processor.parameters.getParameter(juce::String(rabubanj::eqParameterId(busIndex, b, field))))
                {
                    parameterToReset->beginChangeGesture(); parameterToReset->setValueNotifyingHost(parameterToReset->getDefaultValue()); parameterToReset->endChangeGesture();
                }
    };
    for (auto* button : { &bypass, &reset }) addAndMakeVisible(*button);
    addAndMakeVisible(enabled); addAndMakeVisible(type); addAndMakeVisible(graph);
    for (int i = 0; i < 6; ++i) type.addItem(rabubanj::eqTypeNames[static_cast<size_t>(i)], i + 1);
    type.setName("EQ filter type"); type.setTooltip("Bell, shelves, 12 dB/oct cuts or notch");
    for (auto* control : { &frequency, &gain, &q })
    {
        slider(*control, juce::Slider::LinearHorizontal, 80);
        control->setTextBoxStyle(juce::Slider::TextBoxLeft, false, 78, 23);
        addAndMakeVisible(*control);
    }
    frequency.setTextValueSuffix(" Hz"); frequency.setName("EQ frequency"); frequency.setNumDecimalPlacesToDisplay(1);
    gain.setTextValueSuffix(" dB"); gain.setName("EQ gain"); gain.setNumDecimalPlacesToDisplay(2);
    q.setTextValueSuffix(" Q"); q.setName("EQ Q"); q.setNumDecimalPlacesToDisplay(2);
    setBus(bus);
}
EqPanel::~EqPanel() = default;
void EqPanel::setBus(int bus)
{
    if (busIndex == bus) return;
    graph.cancelGesture();
    busIndex = juce::jlimit(0, rabubanj::masterBus, bus);
    bypassAttachment.reset();
    bypassAttachment = std::make_unique<ButtonAttachment>(processor.parameters, "eqBypass" + juce::String(busIndex), bypass);
    bindBand(); repaint();
}
void EqPanel::selectBand(int b)
{
    graph.cancelGesture();
    selectedBand = juce::jlimit(0, rabubanj::eqBandCount - 1, b); bindBand(); graph.repaint();
}
juce::RangedAudioParameter* EqPanel::parameter(const char* field) const
{
    return processor.parameters.getParameter(juce::String(rabubanj::eqParameterId(busIndex, selectedBand, field)));
}
void EqPanel::setBandValue(const char* field, float value)
{
    auto* p = parameter(field); p->setValueNotifyingHost(p->convertTo0to1(value));
}
void EqPanel::bindBand()
{
    frequencyAttachment.reset(); gainAttachment.reset(); qAttachment.reset(); enabledAttachment.reset(); typeAttachment.reset();
    auto id = [&](const char* field) { return juce::String(rabubanj::eqParameterId(busIndex, selectedBand, field)); };
    frequencyAttachment = std::make_unique<SliderAttachment>(processor.parameters, id("freq"), frequency);
    gainAttachment = std::make_unique<SliderAttachment>(processor.parameters, id("gain"), gain);
    qAttachment = std::make_unique<SliderAttachment>(processor.parameters, id("q"), q);
    enabledAttachment = std::make_unique<ButtonAttachment>(processor.parameters, id("on"), enabled);
    typeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.parameters, id("type"), type);
    refresh();
}
void EqPanel::refresh()
{
    const auto settings = processor.getEqSettings(busIndex);
    for (size_t i = 0; i < bands.size(); ++i)
    {
        bands[i].setToggleState(static_cast<int>(i) == selectedBand, juce::dontSendNotification);
        bands[i].setAlpha(settings.bands[i].enabled ? 1.0f : .45f);
    }
    const auto shape = settings.bands[static_cast<size_t>(selectedBand)].type;
    gain.setEnabled(shape < 3); // Gain has no meaning for cuts and notch.
    graph.repaint(); repaint();
}
void EqPanel::paint(juce::Graphics& g)
{
    raisedPanel(g, getLocalBounds().toFloat(), panel, .85f);
    g.setColour(masterPanel ? gold : cyan); g.fillRect(0, 0, getWidth(), 2);
    const juce::String name = busIndex < 8 ? rabubanj::names[static_cast<size_t>(busIndex)] : "Reverb";
    text(g, masterPanel ? "GLOBAL EQ  /  STEREO MASTER" : "CHANNEL EQ  /  " + name.toUpperCase(), { 12, 5, getWidth()-24, 24 }, 12, ink);
    if (getWidth() > 500) text(g, "6-BAND PARAMETRIC", { getWidth()-200, 5, 188, 24 }, 10, muted, juce::Justification::centredRight);
}
void EqPanel::resized()
{
    auto area = getLocalBounds().reduced(10, 6); area.removeFromTop(25);
    auto row = area.removeFromTop(23);
    for (auto& b : bands) { b.setBounds(row.removeFromLeft(23)); row.removeFromLeft(4); }
    reset.setBounds(row.removeFromRight(48)); row.removeFromRight(5); bypass.setBounds(row.removeFromRight(64));
    area.removeFromTop(6);
    auto bottom = area.removeFromBottom(63); graph.setBounds(area);
    bottom.removeFromTop(5);
    auto types = bottom.removeFromTop(23);
    type.setBounds(types.removeFromLeft(juce::jmin(145, getWidth()/3))); types.removeFromLeft(8);
    enabled.setBounds(types.removeFromLeft(80));
    bottom.removeFromTop(6);
    const int width = bottom.getWidth()/3;
    frequency.setBounds(bottom.removeFromLeft(width)); gain.setBounds(bottom.removeFromLeft(width)); q.setBounds(bottom);
    for (auto* s : { &frequency, &gain, &q }) s->setTextBoxStyle(juce::Slider::TextBoxLeft, false, juce::jmin(92, s->getWidth()-26), 23);
}

EqPanel::Graph::Graph(EqPanel& panelToUse) : owner(panelToUse)
{
    setName("EQ response graph"); setMouseCursor(juce::MouseCursor::CrosshairCursor);
    setTooltip("Drag a band: frequency / gain. Mouse wheel: Q. Double-click a band: reset gain. Select filter type below.");
}
juce::Rectangle<float> EqPanel::Graph::plot() const { return getLocalBounds().toFloat().reduced(26, 12).withTrimmedBottom(12); }
juce::Point<float> EqPanel::Graph::point(const rabubanj::EqBand& band) const
{
    const auto r = plot();
    return { r.getX() + r.getWidth()*std::log10(band.frequency/20.0f)/3.0f,
        r.getCentreY() - (band.type < 3 ? band.gain : 0.0f)/24.0f*r.getHeight()*.5f };
}
int EqPanel::Graph::nearest(juce::Point<float> position) const
{
    const auto settings = owner.processor.getEqSettings(owner.busIndex);
    int best = owner.selectedBand; float distance = 18;
    for (int i = 0; i < rabubanj::eqBandCount; ++i)
    {
        const auto d = position.getDistanceFrom(point(settings.bands[static_cast<size_t>(i)]));
        if (d < distance) { distance = d; best = i; }
    }
    return best;
}
void EqPanel::Graph::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat(); const auto r = plot();
    insetPanel(g, bounds);
    const auto settings = owner.processor.getEqSettings(owner.busIndex);
    const auto rate = owner.processor.getDisplaySampleRate();
    auto xFor = [&](float f) { return r.getX() + r.getWidth()*std::log10(f/20.0f)/3.0f; };
    for (int db : { -24, -12, 0, 12, 24 })
    {
        const auto y = r.getCentreY() - static_cast<float>(db)/48.0f*r.getHeight();
        g.setColour(db == 0 ? juce::Colour(0xff506479) : juce::Colour(0xff293c50)); g.drawHorizontalLine(juce::roundToInt(y), r.getX(), r.getRight());
        text(g, juce::String(db), { 0, juce::roundToInt(y)-6, 23, 12 }, 9, muted, juce::Justification::centredRight);
    }
    for (float f : { 20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f })
    {
        if (getWidth() < 450 && (f == 50 || f == 200 || f == 2000 || f == 20000)) continue;
        const auto x = xFor(f); g.setColour(juce::Colour(0xff293c50)); g.drawVerticalLine(juce::roundToInt(x), r.getY(), r.getBottom());
        text(g, f >= 1000 ? juce::String(f/1000, 0)+"k" : juce::String(f, 0), { juce::roundToInt(x)-17, juce::roundToInt(r.getBottom())+3, 34, 12 }, 9, muted, juce::Justification::centred);
    }
    auto response = [&](bool individual)
    {
        juce::Path path;
        for (int pixel = 0; pixel <= static_cast<int>(r.getWidth()); ++pixel)
        {
            const auto f = 20.0 * std::pow(1000.0, pixel / static_cast<double>(r.getWidth()));
            const auto& band = settings.bands[static_cast<size_t>(owner.selectedBand)];
            const auto magnitude = individual ? (band.enabled && !settings.bypass ? rabubanj::eqCoefficients(band, rate).magnitude(f, rate) : 1.0) : rabubanj::eqMagnitude(settings, f, rate);
            const auto db = juce::jlimit(-24.0, 24.0, 20*std::log10(std::max(1e-12, magnitude)));
            const auto x = r.getX()+static_cast<float>(pixel), y = r.getCentreY()-static_cast<float>(db/48)*r.getHeight();
            if (pixel == 0) path.startNewSubPath(x, y); else path.lineTo(x, y);
        }
        return path;
    };
    auto individual = response(true);
    g.setColour(bandColours[static_cast<size_t>(owner.selectedBand)].withAlpha(.5f)); g.strokePath(individual, juce::PathStrokeType(1));
    auto total = response(false), fill = total;
    fill.lineTo(r.getRight(), r.getCentreY()); fill.lineTo(r.getX(), r.getCentreY()); fill.closeSubPath();
    g.setColour(cyan.withAlpha(.10f)); g.fillPath(fill);
    g.setColour(settings.bypass ? muted : ink); g.strokePath(total, juce::PathStrokeType(1.8f));
    for (int i = 0; i < rabubanj::eqBandCount; ++i)
    {
        const auto& band = settings.bands[static_cast<size_t>(i)]; const auto p = point(band);
        const auto colour = bandColours[static_cast<size_t>(i)].withAlpha(band.enabled && !settings.bypass ? 1.0f : .35f);
        if (i == owner.selectedBand) { g.setColour(colour.withAlpha(.16f)); g.fillEllipse(p.x-13, p.y-13, 26, 26); }
        g.setColour(colour); g.fillEllipse(p.x-7, p.y-7, 14, 14);
        text(g, juce::String(i+1), { juce::roundToInt(p.x)-7, juce::roundToInt(p.y)-7, 14, 14 }, 9, dark, juce::Justification::centred);
    }
}
void EqPanel::Graph::mouseDown(const juce::MouseEvent& e)
{
    if (!e.mods.isLeftButtonDown()) return;
    owner.selectBand(nearest(e.position));
    owner.parameter("freq")->beginChangeGesture(); owner.parameter("gain")->beginChangeGesture(); dragging = true;
}
void EqPanel::Graph::moveNode(juce::Point<float> p)
{
    const auto r = plot();
    owner.setBandValue("freq", 20*std::pow(1000.0f, juce::jlimit(0.0f, 1.0f, (p.x-r.getX())/r.getWidth())));
    if (owner.parameter("type")->convertFrom0to1(owner.parameter("type")->getValue()) < 3)
        owner.setBandValue("gain", juce::jlimit(-24.0f, 24.0f, (r.getCentreY()-p.y)/r.getHeight()*48));
    repaint();
}
void EqPanel::Graph::mouseDrag(const juce::MouseEvent& e) { if (dragging) moveNode(e.position); }
void EqPanel::Graph::endGesture()
{
    if (dragging) { owner.parameter("freq")->endChangeGesture(); owner.parameter("gain")->endChangeGesture(); dragging = false; }
}
void EqPanel::Graph::mouseUp(const juce::MouseEvent&) { endGesture(); }
void EqPanel::Graph::mouseDoubleClick(const juce::MouseEvent& e)
{
    endGesture(); owner.selectBand(nearest(e.position)); auto* p = owner.parameter("gain");
    p->beginChangeGesture(); owner.setBandValue("gain", 0); p->endChangeGesture(); repaint();
}
void EqPanel::Graph::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (wheel.deltaY == 0) return;
    owner.selectBand(nearest(e.position)); auto* p = owner.parameter("q");
    const auto value = p->convertFrom0to1(p->getValue()) * std::pow(1.15f, wheel.deltaY > 0 ? 1.0f : -1.0f);
    p->beginChangeGesture(); owner.setBandValue("q", juce::jlimit(.15f, 18.0f, value)); p->endChangeGesture(); repaint();
}
