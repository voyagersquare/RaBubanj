#include "WoodTexture.h"
#if RABUBANJ_HAS_WOOD_TEXTURE
#include <RaBubanjWoodTexture.h>
#endif
namespace rabubanj::ui
{
const juce::Image& woodTexture()
{
#if RABUBANJ_HAS_WOOD_TEXTURE
    static const auto image = []
    {
        int size = 0;
        const auto* bytes = RaBubanjWoodTexture::getNamedResource(RaBubanjWoodTexture::namedResourceList[0], size);
        return juce::ImageFileFormat::loadFrom(bytes, static_cast<size_t>(size));
    }();
#else
    static const juce::Image image;
#endif
    return image;
}
}
