#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
namespace rabubanj::ui
{
// Flat ochre enamel; depth is confined to machined edges and console hardware.
inline const juce::Colour background { 0xffac880b }, panel { 0xffa5800b }, dark { 0xff201e18 }, ink { 0xffffffff }, muted { 0xffffffff }, cyan { 0xff82b9ba }, gold { 0xffe7bb49 };
inline const juce::Colour axis { 0xffcbb77d }, grid { 0xff494438 }, menuFace { 0xff28241c };
inline const std::array<juce::Colour, 6> bandColours { juce::Colour(0xffe8bc56), juce::Colour(0xff9ac985), juce::Colour(0xff80c1c2), juce::Colour(0xffa8b7d1), juce::Colour(0xffc6a5c5), juce::Colour(0xffd79b83) };
inline void raisedPanel(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour base = panel, float depth = .85f)
{
    const auto face = bounds.reduced(2);
    const auto bevel = juce::jlimit(2.0f, 6.0f, 2+depth*1.6f);
    juce::DropShadow(juce::Colours::black.withAlpha(.7f), juce::roundToInt(3+depth*2), { 1, juce::roundToInt(2+depth) }).drawForRectangle(g, face.toNearestInt());
    g.setColour(base.darker(.75f)); g.fillRoundedRectangle(face, 3);
    g.setColour(base.brighter(.42f));
    juce::Path top; top.startNewSubPath(face.getX()+1, face.getBottom()-1); top.lineTo(face.getX()+1, face.getY()+1);
    top.lineTo(face.getRight()-1, face.getY()+1); top.lineTo(face.getRight()-bevel, face.getY()+bevel);
    top.lineTo(face.getX()+bevel, face.getY()+bevel); top.lineTo(face.getX()+bevel, face.getBottom()-bevel); top.closeSubPath(); g.fillPath(top);
    g.setColour(base); g.fillRect(face.reduced(bevel));
    g.setColour(juce::Colour(0xff352c14)); g.drawRoundedRectangle(face, 3, 1);
    g.setColour(base.brighter(.6f).withAlpha(.65f)); g.drawLine(face.getX()+2, face.getY()+1, face.getRight()-2, face.getY()+1, .8f);
}
inline void insetPanel(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    g.setColour(juce::Colour(0xff17150f)); g.fillRoundedRectangle(bounds, 2);
    g.setColour(dark); g.fillRoundedRectangle(bounds.reduced(3), 1);
    g.setColour(juce::Colour(0xff71603a)); g.drawLine(bounds.getX()+2, bounds.getBottom()-1, bounds.getRight()-2, bounds.getBottom()-1, 1.5f);
    g.setColour(juce::Colours::black.withAlpha(.8f)); g.drawLine(bounds.getX()+1, bounds.getY()+1, bounds.getRight()-1, bounds.getY()+1, 2);
}
inline void screw(juce::Graphics& g, float x, float y)
{
    g.setColour(juce::Colours::black.withAlpha(.65f)); g.fillEllipse(x-4, y-3, 9, 9);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffb8bab1), x-3, y-3, juce::Colour(0xff575c56), x+3, y+3, false));
    g.fillEllipse(x-3, y-3, 6, 6); g.setColour(juce::Colour(0xff252823)); g.drawLine(x-2, y+1, x+2, y-1, 1);
}
inline void text(juce::Graphics& g, const juce::String& value, juce::Rectangle<int> bounds, float size = 11, juce::Colour colour = muted, int alignment = juce::Justification::centredLeft)
{
    g.setFont(juce::FontOptions(size));
    g.setColour(juce::Colours::black.withAlpha(.55f)); g.drawText(value, bounds.translated(0, 1), alignment);
    g.setColour(colour); g.drawText(value, bounds, alignment);
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
        g.fillAll(menuFace); g.setColour(juce::Colour(0xffad8839)); g.drawRect(0, 0, width, height, 1);
        g.setColour(juce::Colour(0xff463c27)); g.drawRect(1, 1, width-2, height-2, 1);
    }
};
}
