#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "WoodTexture.h"
namespace rabubanj::ui
{
// Carved relief; the original wood image is embedded unchanged and warmed at paint time.
inline const juce::Colour background { 0xff8b5d34 }, panel { 0xffb68855 }, dark { 0xff201e18 }, ink { 0xffffffff }, muted { 0xffffffff }, cyan { 0xff82b9ba }, gold { 0xffe7bb49 };
inline const juce::Colour axis { 0xffcbb77d }, grid { 0xff494438 }, menuFace { 0xff28241c };
inline const std::array<juce::Colour, 6> bandColours { juce::Colour(0xffe8bc56), juce::Colour(0xff9ac985), juce::Colour(0xff80c1c2), juce::Colour(0xffa8b7d1), juce::Colour(0xffc6a5c5), juce::Colour(0xffd79b83) };
inline void woodSurface(juce::Graphics& g, const juce::Path& path, juce::Colour fallback)
{
    if (woodTexture().isValid())
    {
        const auto bounds = path.getBounds();
        const auto scale = juce::jmax(1344.0f/static_cast<float>(woodTexture().getWidth()), bounds.getRight()/static_cast<float>(woodTexture().getWidth()), bounds.getBottom()/static_cast<float>(woodTexture().getHeight()));
        g.setFillType(juce::FillType(woodTexture(), juce::AffineTransform::scale(scale)));
    }
    else g.setColour(fallback);
    g.fillPath(path);
    // A translucent red-brown finish darkens the grain without replacing the source.
    g.setColour(juce::Colour(0xff6c1e14).withAlpha(.20f));
    g.fillPath(path);
}
inline void woodSurface(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour fallback, float radius = 2)
{ juce::Path path; path.addRoundedRectangle(bounds, radius); woodSurface(g, path, fallback); }
inline const juce::Colour steel { 0xffa2adb9 }, steelDark { 0xff343e4b };
inline void metalSurface(juce::Graphics& g, const juce::Path& path, juce::Colour base = steel)
{
    const auto bounds = path.getBounds();
    juce::ColourGradient sheen(base.darker(.5f), bounds.getX(), bounds.getY(), base.darker(.32f), bounds.getRight(), bounds.getBottom(), false);
    sheen.addColour(.12, base.brighter(.65f)); sheen.addColour(.38, base.brighter(.1f));
    sheen.addColour(.52, base.darker(.22f)); sheen.addColour(.62, base.brighter(.4f));
    g.setGradientFill(sheen); g.fillPath(path);
    juce::Graphics::ScopedSaveState saved(g); g.reduceClipRegion(path);
    for (int line = juce::roundToInt(bounds.getY())+2; line < juce::roundToInt(bounds.getBottom()); line += 2)
    {
        g.setColour(juce::Colours::white.withAlpha(.065f)); g.drawHorizontalLine(line, bounds.getX(), bounds.getRight());
        g.setColour(juce::Colours::black.withAlpha(.05f)); g.drawHorizontalLine(line+1, bounds.getX(), bounds.getRight());
    }
}
inline void metalSurface(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour base = steel, float radius = 2)
{ juce::Path path; path.addRoundedRectangle(bounds, radius); metalSurface(g, path, base); }
inline void raisedMetal(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour base = steel, float rise = 4)
{
    const auto body = bounds.reduced(1), face = body.withTrimmedBottom(rise);
    g.setColour(juce::Colours::black.withAlpha(.7f)); g.fillRoundedRectangle(body.translated(1, 1), 2);
    metalSurface(g, body, steelDark); metalSurface(g, face, base);
    g.setColour(juce::Colour(0xff101722)); g.drawRoundedRectangle(body, 2, 1);
    g.setColour(juce::Colours::white.withAlpha(.7f)); g.drawLine(face.getX()+1, face.getY()+1, face.getRight()-1, face.getY()+1, 1);
    g.setColour(juce::Colours::black.withAlpha(.65f)); g.drawLine(face.getX(), face.getBottom(), face.getRight(), face.getBottom(), 1);
}
inline void flatCaption(juce::Graphics& g, juce::Rectangle<float> bounds,
    juce::Colour face = juce::Colour(0xff142019).withAlpha(.86f),
    juce::Colour edge = juce::Colour(0xffe9d6a5).withAlpha(.45f))
{
    g.setColour(face); g.fillRoundedRectangle(bounds, 2.5f);
    g.setColour(edge); g.drawRoundedRectangle(bounds.reduced(.5f), 2.5f, .8f);
}
inline void raisedPanel(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour base = panel, float depth = .85f)
{
    const auto body = bounds.reduced(1.5f); const auto rise = juce::jlimit(4.0f, 10.0f, 4+depth*2.2f);
    const auto face = body.withTrimmedRight(2).withTrimmedBottom(rise);
    if (depth > .3f) juce::DropShadow(juce::Colours::black.withAlpha(.75f), 9, { 2, 5 }).drawForRectangle(g, body.reduced(2).toNearestInt());
    woodSurface(g, body, base.darker(.7f));
    g.setGradientFill(juce::ColourGradient(juce::Colours::black.withAlpha(.1f), body.getX(), body.getY(), juce::Colours::black.withAlpha(.65f), body.getRight(), body.getBottom(), false)); g.fillRoundedRectangle(body, 3);
    woodSurface(g, face, base);
    g.setColour(juce::Colour(0xffffe4b2).withAlpha(.6f)); g.drawRoundedRectangle(face.reduced(.6f), 2, 1.2f);
    g.setColour(juce::Colour(0xff3a1c0e)); g.drawRoundedRectangle(body, 3, 1);
    g.setColour(juce::Colours::black.withAlpha(.55f)); g.drawLine(face.getX()+2, face.getBottom(), face.getRight()-1, face.getBottom(), 1.8f);
    g.setColour(juce::Colour(0xffe4b579).withAlpha(.45f)); g.drawLine(body.getX()+3, body.getBottom()-1.5f, body.getRight()-3, body.getBottom()-1.5f, .8f);
}
inline void insetPanel(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    woodSurface(g, bounds, juce::Colour(0xff6b4225));
    const auto well = bounds.reduced(4);
    woodSurface(g, well, dark); g.setColour(juce::Colours::black.withAlpha(.88f)); g.fillRoundedRectangle(well, 2);
    g.setColour(juce::Colour(0xff2c160d)); g.drawRoundedRectangle(bounds.reduced(1), 2, 2);
    g.setColour(juce::Colours::black.withAlpha(.85f)); g.drawLine(well.getX(), well.getY(), well.getRight(), well.getY(), 3);
    g.setColour(juce::Colour(0xffebbd86).withAlpha(.65f)); g.drawLine(bounds.getX()+2, bounds.getBottom()-1, bounds.getRight()-2, bounds.getBottom()-1, 1);
}
inline void screw(juce::Graphics& g, float x, float y)
{
    g.setColour(juce::Colours::black.withAlpha(.65f)); g.fillEllipse(x-3, y-1, 7, 7);
    juce::Path plug; plug.addEllipse(x-3, y-3, 6, 6); metalSurface(g, plug, steel);
    g.setColour(juce::Colour(0xff27303c)); g.strokePath(plug, juce::PathStrokeType(.8f));
    g.drawLine(x-2, y+1, x+2, y-1, .9f);
}
inline void text(juce::Graphics& g, const juce::String& value, juce::Rectangle<int> bounds, float size = 11, juce::Colour colour = muted, int alignment = juce::Justification::centredLeft, bool bold = false)
{
    const auto font = juce::Font(juce::FontOptions(size)); g.setFont(bold ? font.boldened() : font);
    g.setColour(juce::Colours::black.withAlpha(.8f));
    for (const auto offset : { juce::Point<int>(-1, 0), { 1, 0 }, { 0, -1 }, { 0, 1 } }) g.drawText(value, bounds.translated(offset.x, offset.y), alignment);
    g.setColour(colour); g.drawText(value, bounds, alignment);
}
inline void woodText(juce::Graphics& g, const juce::String& value, juce::Rectangle<int> bounds,
    float size = 12, int alignment = juce::Justification::centredLeft, bool bold = true)
{
    const auto font = juce::Font(juce::FontOptions(size)); g.setFont(bold ? font.boldened() : font);
    g.setColour(juce::Colour(0xffffe9c4).withAlpha(.6f)); g.drawText(value, bounds.translated(0, 1), alignment);
    g.setColour(juce::Colours::black); g.drawText(value, bounds, alignment);
}
inline void slider(juce::Slider& s, juce::Slider::SliderStyle style, int textWidth = 46)
{
    s.setSliderStyle(style); s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, 20);
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff493b1d));
    s.setColour(juce::Slider::textBoxBackgroundColourId, dark); s.setColour(juce::Slider::textBoxTextColourId, ink);
    s.setColour(juce::Slider::trackColourId, cyan); s.setColour(juce::Slider::thumbColourId, cyan);
    s.setColour(juce::Slider::rotarySliderFillColourId, cyan); s.setColour(juce::Slider::rotarySliderOutlineColourId, dark);
}
// Kept alive by the async menu callback if a plugin window closes with its menu open.
class EqMenuLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    EqMenuLookAndFeel()
    {
        setColour(juce::PopupMenu::backgroundColourId, menuFace); setColour(juce::PopupMenu::textColourId, juce::Colour(0xffeee2be));
        setColour(juce::PopupMenu::headerTextColourId, axis); setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff795e1a));
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(0xffffecc1));
    }
    juce::Font getPopupMenuFont() override { return juce::Font(juce::FontOptions(14.0f)); }
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area, bool separator, bool active, bool highlighted, bool ticked, bool submenu,
        const juce::String& label, const juce::String& shortcut, const juce::Drawable* icon, const juce::Colour* colour) override
    {
        juce::LookAndFeel_V4::drawPopupMenuItem(g, area, separator, active, highlighted, ticked, submenu, label, shortcut, icon, colour);
        if (icon != nullptr && ticked)
        {
            const auto x = static_cast<float>(area.getRight()-16), y = static_cast<float>(area.getCentreY());
            g.setColour(gold); g.drawLine(x-4, y, x-1, y+3, 1.5f); g.drawLine(x-1, y+3, x+5, y-4, 1.5f);
        }
    }
    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        woodSurface(g, { 0, 0, static_cast<float>(width), static_cast<float>(height) }, menuFace); g.setColour(juce::Colours::black.withAlpha(.88f)); g.fillAll(); g.setColour(juce::Colour(0xffad8839)); g.drawRect(0, 0, width, height, 1);
        g.setColour(juce::Colour(0xff463c27)); g.drawRect(1, 1, width-2, height-2, 1);
    }
};
}
