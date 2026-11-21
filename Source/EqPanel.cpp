#include "EqPanel.h"
using namespace rabubanj::ui;

EqPanel::EqPanel(RaBubanjProcessor& p, int bus, bool isMaster) : processor(p), masterPanel(isMaster)
{
    setName(isMaster ? "Global EQ" : "Channel EQ");
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
    addAndMakeVisible(graph);
    setBus(bus);
}
EqPanel::~EqPanel() = default;
void EqPanel::setBus(int bus)
{
    if (busIndex == bus) return;
    graph.cancelGesture(); busIndex = juce::jlimit(0, rabubanj::masterBus, bus);
    bypassAttachment.reset();
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters, "eqBypass"+juce::String(busIndex), bypass);
    processor.getSpectrumTap(busIndex).discard(); graph.resetSpectrum();
    refresh(); repaint();
}
void EqPanel::selectBand(int b)
{
    graph.cancelGesture(); selectedBand = juce::jlimit(0, rabubanj::eqBandCount-1, b); refresh(); graph.repaint();
}
juce::RangedAudioParameter* EqPanel::parameter(const char* field) const
{
    return processor.parameters.getParameter(juce::String(rabubanj::eqParameterId(busIndex, selectedBand, field)));
}
void EqPanel::setBandValue(const char* field, float value)
{
    auto* p = parameter(field); p->setValueNotifyingHost(p->convertTo0to1(value));
}
juce::PopupMenu EqPanel::makeBandContextMenu(int band) const
{
    const auto settings = processor.getEqSettings(busIndex);
    const auto& value = settings.bands[static_cast<size_t>(juce::jlimit(0, rabubanj::eqBandCount-1, band))];
    juce::PopupMenu menu, shapes, gains;
    for (auto* m : { &menu, &shapes, &gains }) m->setLookAndFeel(menuLook.get());
    menu.addSectionHeader("Band "+juce::String(band+1)+" / "+rabubanj::eqTypeNames[static_cast<size_t>(value.type)]);
    menu.addItem(-1, juce::String(value.frequency, 1)+" Hz   "+(value.gain > 0 ? "+" : "")+juce::String(value.gain, 1)+" dB   Q "+juce::String(value.q, 2), false);
    menu.addSeparator();
    menu.addItem(ToggleBand, "Band enabled", true, value.enabled);
    for (int shape = 0; shape < 6; ++shape)
    {
        juce::PopupMenu::Item item(rabubanj::eqTypeNames[static_cast<size_t>(shape)]);
        item.itemID = ShapeBase+shape; item.isTicked = value.type == shape;
        auto icon = std::make_unique<juce::DrawablePath>(); juce::Path path;
        rabubanj::EqBand example { 1000, 9, .7071f, shape, true };
        const auto coefficients = rabubanj::eqCoefficients(example, 48000);
        for (int x = 0; x <= 32; ++x)
        {
            const auto frequency = 50*std::pow(320.0, x/32.0);
            const auto db = juce::jlimit(-12.0, 12.0, 20*std::log10(std::max(1e-9, coefficients.magnitude(frequency, 48000))));
            const auto y = 12-static_cast<float>(db)*.75f;
            if (x == 0) path.startNewSubPath(0, y); else path.lineTo(static_cast<float>(x), y);
        }
        icon->setPath(path); icon->setFill(juce::FillType(juce::Colours::transparentBlack)); icon->setStrokeFill(juce::FillType(gold)); icon->setStrokeType(juce::PathStrokeType(1.8f));
        item.image = std::move(icon); shapes.addItem(std::move(item));
    }
    menu.addSubMenu("Curve shape", shapes);
    for (size_t i = 0; i < gainChoices.size(); ++i)
    {
        const auto db = gainChoices[i];
        gains.addItem(GainBase+static_cast<int>(i), (db > 0 ? "+" : "")+juce::String(db)+" dB", true, std::abs(value.gain-static_cast<float>(db)) < .01f);
    }
    menu.addSubMenu("Gain (dB)", gains, value.type < 3);
    menu.addItem(InvertGain, "Invert gain", value.type < 3);
    return menu;
}
void EqPanel::applyBandContextAction(int bus, int band, int command)
{
    if (bus < 0 || bus >= rabubanj::eqBusCount || band < 0 || band >= rabubanj::eqBandCount) return;
    const auto current = processor.getEqSettings(bus).bands[static_cast<size_t>(band)];
    const char* field = nullptr; float value = 0;
    if (command == ToggleBand) { field = "on"; value = current.enabled ? 0.0f : 1.0f; }
    else if (command == InvertGain && current.type < 3) { field = "gain"; value = -current.gain; }
    else if (command >= ShapeBase && command < ShapeBase+6) { field = "type"; value = static_cast<float>(command-ShapeBase); }
    else if (command >= GainBase && command < GainBase+static_cast<int>(gainChoices.size()) && current.type < 3)
    { field = "gain"; value = static_cast<float>(gainChoices[static_cast<size_t>(command-GainBase)]); }
    if (field == nullptr) return;
    auto* p = processor.parameters.getParameter(juce::String(rabubanj::eqParameterId(bus, band, field)));
    p->beginChangeGesture(); p->setValueNotifyingHost(p->convertTo0to1(value)); p->endChangeGesture(); refresh();
}
void EqPanel::showBandContextMenu(juce::Component& target, juce::Point<int> screenPosition)
{
    if (!target.isShowing()) return;
    auto menu = makeBandContextMenu(selectedBand); const auto bus = busIndex, band = selectedBand;
    const juce::Component::SafePointer<EqPanel> safe(this); auto keepLookAlive = menuLook;
    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&target).withTargetScreenArea({ screenPosition.x, screenPosition.y, 1, 1 }).withMinimumWidth(270).withStandardItemHeight(27),
        [safe, bus, band, keepLookAlive](int command)
        { juce::ignoreUnused(keepLookAlive); if (safe) safe->applyBandContextAction(bus, band, command); });
}
void EqPanel::refresh()
{
    const auto settings = processor.getEqSettings(busIndex);
    const auto& band = settings.bands[static_cast<size_t>(selectedBand)];
    graph.setTooltip("Band "+juce::String(selectedBand+1)+": "+juce::String(band.frequency, 1)+" Hz / "+juce::String(band.gain, 2)
        +" dB / Q "+juce::String(band.q, 2)+(band.enabled ? " / ON" : " / OFF")
        +". Live spectrum: -90 to 0 dBFS, post-EQ (master: stereo output). Drag: frequency/gain; wheel: Q; right-click: curve shape / gain menu; Alt-click: on/off; double-click: reset gain.");
    graph.updateSpectrum(); graph.repaint(); repaint();
}
void EqPanel::paint(juce::Graphics& g)
{
    raisedPanel(g, getLocalBounds().toFloat(), panel, 1.9f);
    g.setColour(masterPanel ? gold : cyan); g.fillRect(0, 0, getWidth(), 2);
    const juce::String name = busIndex < 8 ? rabubanj::padNames[static_cast<size_t>(busIndex)] : "Reverb";
    woodText(g, masterPanel ? "GLOBAL EQ  /  STEREO MASTER" : "CHANNEL EQ  /  "+name.toUpperCase(), { 12, 5, getWidth()-24, 24 }, 14);
    if (getWidth() > 500) woodText(g, "6-BAND PARAMETRIC", { getWidth()-200, 5, 188, 24 }, 12, juce::Justification::centredRight);
}
void EqPanel::resized()
{
    auto area = getLocalBounds().reduced(10, 6); area.removeFromTop(25);
    auto row = area.removeFromTop(23);
    reset.setBounds(row.removeFromRight(48)); row.removeFromRight(5); bypass.setBounds(row.removeFromRight(64));
    area.removeFromTop(6);
    graph.setBounds(area.withTrimmedBottom(4));
}

EqPanel::Graph::Graph(EqPanel& panelToUse) : owner(panelToUse)
{
    setName("EQ response graph"); setMouseCursor(juce::MouseCursor::CrosshairCursor);
    setTooltip("Drag a band: frequency / gain. Mouse wheel: Q. Double-click a band: reset gain. Right-click a node for curve shape and gain.");
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
    insetPanel(g, bounds, true);
    const auto settings = owner.processor.getEqSettings(owner.busIndex);
    const auto rate = owner.processor.getDisplaySampleRate();
    auto xFor = [&](float f) { return r.getX() + r.getWidth()*std::log10(f/20.0f)/3.0f; };
    for (int db : { -24, -12, 0, 12, 24 })
    {
        const auto y = r.getCentreY() - static_cast<float>(db)/48.0f*r.getHeight();
        g.setColour(db == 0 ? juce::Colour(0xff80704a) : grid); g.drawHorizontalLine(juce::roundToInt(y), r.getX(), r.getRight());
        text(g, juce::String(db), { 0, juce::roundToInt(y)-6, 23, 12 }, 9, axis, juce::Justification::centredRight);
    }
    for (float f : { 20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f })
    {
        if (getWidth() < 450 && (f == 50 || f == 200 || f == 2000 || f == 20000)) continue;
        const auto x = xFor(f); g.setColour(grid); g.drawVerticalLine(juce::roundToInt(x), r.getY(), r.getBottom());
        text(g, f >= 1000 ? juce::String(f/1000, 0)+"k" : juce::String(f, 0), { juce::roundToInt(x)-17, juce::roundToInt(r.getBottom())+3, 34, 12 }, 9, axis, juce::Justification::centred);
    }
    {
        const auto colour = owner.masterPanel ? gold : cyan;
        juce::Path trace;
        for (int pixel = 0; pixel <= static_cast<int>(r.getWidth()); ++pixel)
        {
            const auto frequency = 20.0f*std::pow(1000.0f, static_cast<float>(pixel)/r.getWidth());
            const auto db = juce::jlimit(-90.0f, 0.0f, spectrum.magnitudeAt(frequency, rate));
            const auto y = r.getBottom()-(db+90)/90*r.getHeight();
            if (pixel == 0) trace.startNewSubPath(r.getX(), y); else trace.lineTo(r.getX()+static_cast<float>(pixel), y);
        }
        auto fill = trace; fill.lineTo(r.getRight(), r.getBottom()); fill.lineTo(r.getX(), r.getBottom()); fill.closeSubPath();
        g.setColour(colour.withAlpha(.16f)); g.fillPath(fill); g.setColour(colour.withAlpha(.65f)); g.strokePath(trace, juce::PathStrokeType(1.1f));
        for (int db : { -12, -36, -60, -84 })
            text(g, juce::String(db), { juce::roundToInt(r.getRight())+2, juce::roundToInt(r.getBottom()-static_cast<float>(db+90)/90*r.getHeight())-5, 23, 10 }, 8, axis, juce::Justification::centredLeft);
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
    g.setColour(gold.withAlpha(.08f)); g.fillPath(fill);
    g.setColour(settings.bypass ? axis : gold); g.strokePath(total, juce::PathStrokeType(1.8f));
    for (int i = 0; i < rabubanj::eqBandCount; ++i)
    {
        const auto& band = settings.bands[static_cast<size_t>(i)]; const auto p = point(band);
        const auto colour = bandColours[static_cast<size_t>(i)].withAlpha(band.enabled && !settings.bypass ? 1.0f : .35f);
        if (i == owner.selectedBand) { g.setColour(colour.withAlpha(.16f)); g.fillEllipse(p.x-13, p.y-13, 26, 26); }
        g.setColour(juce::Colours::black.withAlpha(.65f)); g.fillEllipse(p.x-7, p.y-4, 15, 15);
        juce::Path cap; cap.addEllipse(p.x-7, p.y-8, 14, 14); metalSurface(g, cap, band.enabled && !settings.bypass ? steel : steelDark);
        g.setColour(colour); g.strokePath(cap, juce::PathStrokeType(2));
        text(g, juce::String(i+1), { juce::roundToInt(p.x)-7, juce::roundToInt(p.y)-8, 14, 14 }, 9, ink, juce::Justification::centred);
    }
}
void EqPanel::Graph::mouseDown(const juce::MouseEvent& e)
{
    if (!e.mods.isLeftButtonDown() && !e.mods.isRightButtonDown()) return;
    if (e.mods.isRightButtonDown())
    {
        const auto band = nearest(e.position);
        if (e.position.getDistanceFrom(point(owner.processor.getEqSettings(owner.busIndex).bands[static_cast<size_t>(band)])) > 18) return;
        owner.selectBand(band); owner.showBandContextMenu(*this, e.getScreenPosition()); return;
    }
    owner.selectBand(nearest(e.position));
    if (e.mods.isAltDown())
    {
        auto* enabled = owner.parameter("on"); enabled->beginChangeGesture();
        owner.setBandValue("on", enabled->getValue() >= .5f ? 0.0f : 1.0f); enabled->endChangeGesture();
        owner.refresh(); return;
    }
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
    if (e.mods.isRightButtonDown()) return;
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
