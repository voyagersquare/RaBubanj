#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
namespace rabubanj::ui
{
inline const juce::Colour background { 0xff1b1e23 }, panel { 0xff2b3037 }, dark { 0xff14171b }, ink { 0xffdce4e9 }, muted { 0xff8d99a3 }, cyan { 0xff64b4c4 }, gold { 0xffdfb16d };
inline const std::array<juce::Colour, 6> bandColours { juce::Colour(0xffe6b66a), juce::Colour(0xff8cce7f), juce::Colour(0xff64c7d2), juce::Colour(0xff9aaff1), juce::Colour(0xffd999d5), juce::Colour(0xffdf8d83) };
inline void text(juce::Graphics& g, const juce::String& value, juce::Rectangle<int> bounds, float size = 11, juce::Colour colour = muted, int alignment = juce::Justification::centredLeft)
{
    g.setFont(juce::FontOptions(size)); g.setColour(colour); g.drawText(value, bounds, alignment);
}
inline void slider(juce::Slider& s, juce::Slider::SliderStyle style, int textWidth = 46)
{
    s.setSliderStyle(style); s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, 20);
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    s.setColour(juce::Slider::textBoxBackgroundColourId, dark); s.setColour(juce::Slider::textBoxTextColourId, ink);
    s.setColour(juce::Slider::trackColourId, cyan); s.setColour(juce::Slider::thumbColourId, cyan);
    s.setColour(juce::Slider::rotarySliderFillColourId, cyan); s.setColour(juce::Slider::rotarySliderOutlineColourId, dark);
}
}
