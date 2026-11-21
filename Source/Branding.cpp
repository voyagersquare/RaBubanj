#include "Branding.h"
#include <RaBubanjBrandFont.h>
namespace rabubanj::ui
{
void drawBrand(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Cache actual glyph outlines, including swashes, so fitting never clips the script.
    static const auto lettering = []
    {
        int size = 0;
        const auto* bytes = RaBubanjBrandFont::getNamedResource(RaBubanjBrandFont::namedResourceList[0], size);
        const auto face = juce::Typeface::createSystemTypefaceFor(bytes, static_cast<size_t>(size));
        juce::GlyphArrangement glyphs;
        glyphs.addLineOfText(juce::Font(juce::FontOptions(face).withHeight(100)), "RaBubanj", 0, 0);
        juce::Path path; glyphs.createPath(path); return path;
    }();
    g.setColour(juce::Colour(0xff7d0e1b));
    g.fillPath(lettering, lettering.getTransformToScaleToFit(bounds, true, juce::Justification::centredLeft));
}
}
