#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
namespace rabubanj::ui
{
inline const juce::Colour background { 0xff081321 }, panel { 0xff182e49 }, dark { 0xff070f19 }, ink { 0xffe0e3df }, muted { 0xffbac6cf }, cyan { 0xff76b4c9 }, gold { 0xffcdb181 };
inline const std::array<juce::Colour, 6> bandColours { juce::Colour(0xffe6b66a), juce::Colour(0xff8cce7f), juce::Colour(0xff64c7d2), juce::Colour(0xff9aaff1), juce::Colour(0xffd999d5), juce::Colour(0xffdf8d83) };

// A deterministic, cached finish: chipped enamel, abrasion, grime and settled dust.
// Built on the UI thread once; no disk access or work in the audio callback.
inline const juce::Image& wearTexture()
{
    static const juce::Image texture = []
    {
        constexpr int edge = 768;
        juce::Image image(juce::Image::ARGB, edge, edge, true);
        juce::Random random(0x52614275);
        auto hash = [](int x, int y, int salt)
        {
            auto h = static_cast<juce::uint32>(x)*374761393u + static_cast<juce::uint32>(y)*668265263u + static_cast<juce::uint32>(salt)*1442695041u;
            h = (h ^ (h >> 13u))*1274126177u; h ^= h >> 16u;
            return static_cast<float>(h & 65535u)/65535.0f;
        };
        auto noise = [&](float x, float y, int salt)
        {
            const int ix = static_cast<int>(std::floor(x)), iy = static_cast<int>(std::floor(y));
            auto fx = x-static_cast<float>(ix), fy = y-static_cast<float>(iy);
            fx = fx*fx*(3-2*fx); fy = fy*fy*(3-2*fy);
            const auto upper = hash(ix, iy, salt)*(1-fx)+hash(ix+1, iy, salt)*fx;
            const auto lower = hash(ix, iy+1, salt)*(1-fx)+hash(ix+1, iy+1, salt)*fx;
            return upper*(1-fy)+lower*fy;
        };
        {
            juce::Image::BitmapData pixels(image, juce::Image::BitmapData::writeOnly);
            for (int y = 0; y < edge; ++y) for (int x = 0; x < edge; ++x)
            {
                const auto px = static_cast<float>(x), py = static_cast<float>(y);
                const auto grain = hash(x, y, 31);
                const auto erosion = noise(px/94, py/94, 7)*.52f + noise(px/29, py/29, 13)*.28f
                    + noise(px/9, py/9, 29)*.14f + grain*.06f;
                const auto exposed = juce::jlimit(0.0f, 1.0f, (erosion-.46f)*13);
                const auto oxideField = noise(px/73+4, py/73+7, 17)*.7f + noise(px/16, py/16, 43)*.3f;
                const auto oxide = juce::jlimit(0.0f, 1.0f, (oxideField-.41f)*5);
                const auto steel = juce::Colour(0xff9aa5a7).interpolatedWith(juce::Colour(0xff3a4d5b), grain*.7f);
                const auto rust = juce::Colour(0xff48241c).interpolatedWith(juce::Colour(0xffbb6a36), grain*.75f + noise(px/4, py/4, 57)*.25f);
                auto colour = steel.interpolatedWith(rust, oxide);
                // Fine pits remain dark within both bare metal and oxidised patches.
                if (grain < .08f) colour = colour.darker(.7f);
                const auto dust = juce::Colour(0xffb4af9c).interpolatedWith(juce::Colour(0xff010711), grain);
                const auto alpha = juce::jlimit(0.0f, .96f, .06f+exposed*.9f);
                pixels.setPixelColour(x, y, dust.interpolatedWith(colour, exposed).withAlpha(alpha));
            }
        }
        juce::Graphics g(image);
        for (int i = 0; i < 125; ++i)
        {
            const float x = random.nextFloat()*edge, y = random.nextFloat()*edge, radius = 7+random.nextFloat()*55;
            const auto dirt = i % 3 == 0 ? juce::Colour(0xff00040b) : juce::Colour(0xffa39c85);
            g.setGradientFill(juce::ColourGradient(dirt.withAlpha(.08f+random.nextFloat()*.15f), x, y,
                dirt.withAlpha(0.0f), x+radius, y, true));
            g.fillEllipse(x-radius, y-radius, radius*2, radius*2);
        }
        for (int i = 0; i < 420; ++i)
        {
            const float x = random.nextFloat()*edge, y = random.nextFloat()*edge;
            const float length = 2+std::pow(random.nextFloat(), 2.0f)*103;
            const float dy = (random.nextFloat()-.5f)*length*.7f;
            g.setColour(juce::Colours::black.withAlpha(.3f+random.nextFloat()*.25f));
            g.drawLine(x, y+1, x+length, y+dy+1, 1.6f);
            g.setColour(juce::Colour(0xffacb9bd).withAlpha(.12f+random.nextFloat()*.40f));
            g.drawLine(x, y, x+length, y+dy, .4f+random.nextFloat()*.7f);
        }
        for (int i = 0; i < 155; ++i)
        {
            const float x = random.nextFloat()*edge, y = random.nextFloat()*edge;
            const float radius = 1+random.nextFloat()*10;
            juce::Path chip;
            for (int corner = 0; corner < 7; ++corner)
            {
                const auto angle = static_cast<float>(corner)*juce::MathConstants<float>::twoPi/7;
                const auto reach = radius*(.3f+random.nextFloat());
                const auto px = x+std::cos(angle)*reach*2.2f, py = y+std::sin(angle)*reach*.5f;
                if (corner == 0) chip.startNewSubPath(px, py); else chip.lineTo(px, py);
            }
            chip.closeSubPath();
            g.setColour(juce::Colour(0xff020710).withAlpha(.65f)); g.strokePath(chip, juce::PathStrokeType(1.8f));
            g.setColour(juce::Colour(0xffa7afb0).withAlpha(.25f+random.nextFloat()*.25f)); g.fillPath(chip);
        }
        for (int i = 0; i < 6800; ++i)
        {
            const float x = random.nextFloat()*edge, y = random.nextFloat()*edge;
            g.setColour(juce::Colour(0xffd0c6ac).withAlpha(.07f+random.nextFloat()*.20f));
            g.fillEllipse(x, y, .4f+random.nextFloat()*1.4f, .4f+random.nextFloat()*1.1f);
        }
        return image;
    }();
    return texture;
}
inline void wear(juce::Graphics& g, juce::Rectangle<float> bounds, float opacity = .8f, int offset = 0)
{
    juce::Graphics::ScopedSaveState saved(g);
    juce::Path clip; clip.addRoundedRectangle(bounds, 3); g.reduceClipRegion(clip);
    g.setTiledImageFill(wearTexture(), offset, offset/3, opacity); g.fillRect(bounds);
}
inline void raisedPanel(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour base = panel, float wearAmount = .85f)
{
    auto face = bounds.reduced(2);
    juce::DropShadow(juce::Colours::black.withAlpha(.85f), 4, { 1, 3 }).drawForRectangle(g, face.toNearestInt());
    g.setGradientFill(juce::ColourGradient(base.brighter(.25f), face.getTopLeft(), base.darker(.45f), face.getBottomRight(), false));
    g.fillRoundedRectangle(face, 4); wear(g, face, wearAmount, juce::roundToInt(bounds.getX()*5+bounds.getY()*3));
    g.setColour(juce::Colour(0xff020810)); g.drawRoundedRectangle(face, 4, 1.8f);
    auto inset = face.reduced(1.4f);
    juce::Path light; light.startNewSubPath(inset.getX()+3, inset.getBottom()-3); light.lineTo(inset.getX(), inset.getY()+3);
    light.quadraticTo(inset.getX(), inset.getY(), inset.getX()+3, inset.getY()); light.lineTo(inset.getRight()-3, inset.getY());
    g.setColour(juce::Colour(0xff91a5b7).withAlpha(.52f)); g.strokePath(light, juce::PathStrokeType(1));
    g.setColour(juce::Colours::black.withAlpha(.7f)); g.drawLine(face.getX()+4, face.getBottom()-1, face.getRight()-3, face.getBottom()-1, 1.5f);
    // Exposed metal at battered edges.
    g.setColour(juce::Colour(0xffb8b9ae).withAlpha(.38f));
    for (int i = 0; i < static_cast<int>(face.getWidth())/26; ++i)
    {
        const auto x = face.getX()+8+static_cast<float>(i*26);
        const auto y = i%3 == 0 ? face.getY()+1 : face.getBottom()-2;
        g.drawLine(x, y, x+static_cast<float>(2+(i*7)%13), y-.4f, .7f);
    }
}
inline void insetPanel(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    g.setColour(juce::Colour(0xff02060d)); g.fillRoundedRectangle(bounds, 3);
    g.setGradientFill(juce::ColourGradient(dark, bounds.getTopLeft(), juce::Colour(0xff0c1b2b), bounds.getBottomRight(), false));
    g.fillRoundedRectangle(bounds.reduced(2), 2);
    g.setColour(juce::Colour(0xff52677d).withAlpha(.5f)); g.drawLine(bounds.getX()+2, bounds.getBottom()-1, bounds.getRight()-2, bounds.getBottom()-1);
}
inline void screw(juce::Graphics& g, float x, float y)
{
    g.setColour(juce::Colours::black.withAlpha(.8f)); g.fillEllipse(x-4, y-3, 9, 9);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff8c9aa3), x-3, y-3, juce::Colour(0xff263341), x+3, y+3, false));
    g.fillEllipse(x-3, y-3, 6, 6); g.setColour(juce::Colour(0xff111820)); g.drawLine(x-2, y+1, x+2, y-1, 1);
}
inline void text(juce::Graphics& g, const juce::String& value, juce::Rectangle<int> bounds, float size = 11, juce::Colour colour = muted, int alignment = juce::Justification::centredLeft)
{
    g.setFont(juce::FontOptions(size));
    g.setColour(juce::Colours::black.withAlpha(.92f));
    if (size <= 13)
    {
        for (auto offset : { juce::Point<int>(-1, 0), juce::Point<int>(1, 0), juce::Point<int>(0, -1), juce::Point<int>(0, 1) })
            g.drawText(value, bounds.translated(offset.x, offset.y), alignment);
    }
    else g.drawText(value, bounds.translated(1, 2), alignment);
    g.setColour(colour); g.drawText(value, bounds, alignment);
}
inline void slider(juce::Slider& s, juce::Slider::SliderStyle style, int textWidth = 46)
{
    s.setSliderStyle(style); s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textWidth, 20);
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff050b12));
    s.setColour(juce::Slider::textBoxBackgroundColourId, dark); s.setColour(juce::Slider::textBoxTextColourId, ink);
    s.setColour(juce::Slider::trackColourId, cyan); s.setColour(juce::Slider::thumbColourId, cyan);
    s.setColour(juce::Slider::rotarySliderFillColourId, cyan); s.setColour(juce::Slider::rotarySliderOutlineColourId, dark);
}
}
