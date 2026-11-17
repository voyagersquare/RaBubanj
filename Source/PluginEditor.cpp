#include "PluginEditor.h"

namespace
{
const juce::Colour background { 0xff10151e }, panel { 0xff1a2230 }, ink { 0xffe9edf3 }, accent { 0xffedb76a }, muted { 0xff8e9bae };
void styleSlider(juce::Slider& slider, juce::Slider::SliderStyle style, const juce::String& suffix = {})
{
    slider.setSliderStyle(style);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 78, 23);
    slider.setTextValueSuffix(suffix);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, background);
    slider.setColour(juce::Slider::textBoxTextColourId, ink);
    slider.setColour(juce::Slider::trackColourId, accent);
    slider.setColour(juce::Slider::thumbColourId, accent);
    slider.setColour(juce::Slider::rotarySliderFillColourId, accent);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, background);
}
}

RaBubanjEditor::RaBubanjEditor(RaBubanjProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    look.setColour(juce::TextButton::buttonColourId, panel.brighter(.2f));
    look.setColour(juce::TextButton::buttonOnColourId, accent);
    look.setColour(juce::TextButton::textColourOffId, ink);
    look.setColour(juce::TextButton::textColourOnId, background);
    look.setColour(juce::Slider::backgroundColourId, background);
    setLookAndFeel(&look);
    for (int d = 0; d < rabubanj::drumCount; ++d)
    {
        auto& channel = channels[static_cast<size_t>(d)];
        const auto id = juce::String(d);
        channel.pad.setButtonText(juce::String(rabubanj::names[static_cast<size_t>(d)]) + "\n" + juce::String(rabubanj::notes[static_cast<size_t>(d)]));
        channel.pad.setTooltip("Preview " + juce::String(rabubanj::names[static_cast<size_t>(d)]) + "; MIDI note " + juce::String(rabubanj::notes[static_cast<size_t>(d)]));
        channel.pad.onClick = [this, d] { processor.audition(d); };
        addAndMakeVisible(channel.pad);
        channel.mute.setButtonText("MUTE");
        channel.mute.setClickingTogglesState(true);
        channel.mute.setTooltip("Mute this channel, including its room feed");
        addAndMakeVisible(channel.mute);
        styleSlider(channel.gain, juce::Slider::LinearVertical, " dB");
        channel.gain.setName(juce::String(rabubanj::names[static_cast<size_t>(d)]) + " level");
        channel.gain.setDoubleClickReturnValue(true, 0);
        addAndMakeVisible(channel.gain);
        styleSlider(channel.pan, juce::Slider::RotaryHorizontalVerticalDrag);
        channel.pan.setName(juce::String(rabubanj::names[static_cast<size_t>(d)]) + " pan");
        channel.pan.setDoubleClickReturnValue(true, 0);
        channel.pan.setTooltip("Pan: -1 left, 0 centre, +1 right");
        addAndMakeVisible(channel.pan);
        channel.gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "gain" + id, channel.gain);
        channel.panAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "pan" + id, channel.pan);
        channel.muteAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters, "mute" + id, channel.mute);
    }
    styleSlider(master, juce::Slider::LinearHorizontal, " dB");
    master.setName("Master level");
    master.setDoubleClickReturnValue(true, -8);
    addAndMakeVisible(master);
    styleSlider(room, juce::Slider::LinearHorizontal);
    room.setName("Room amount");
    room.setDoubleClickReturnValue(true, .12);
    addAndMakeVisible(room);
    masterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "master", master);
    roomAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "room", room);
    setResizable(true, true);
    setResizeLimits(960, 660, 1600, 1100);
    setSize(1120, 740);
    startTimerHz(30);
}

RaBubanjEditor::~RaBubanjEditor() { stopTimer(); setLookAndFeel(nullptr); }

void RaBubanjEditor::resized()
{
    auto area = getLocalBounds().reduced(28);
    area.removeFromTop(92);
    padArea = area.removeFromTop(106);
    area.removeFromTop(32);
    footerArea = area.removeFromBottom(90);
    area.removeFromBottom(16);
    mixerArea = area;
    const int width = padArea.getWidth() / rabubanj::drumCount;
    for (int d = 0; d < rabubanj::drumCount; ++d)
    {
        auto& channel = channels[static_cast<size_t>(d)];
        channel.pad.setBounds(padArea.getX() + d * width + 5, padArea.getY() + 6, width - 10, 90);
        auto strip = juce::Rectangle<int>(mixerArea.getX() + d * width, mixerArea.getY(), width, mixerArea.getHeight()).reduced(10, 12);
        strip.removeFromTop(22);
        channel.mute.setBounds(strip.removeFromBottom(28).reduced(8, 0));
        strip.removeFromBottom(8);
        channel.pan.setBounds(strip.removeFromBottom(86));
        channel.gain.setBounds(strip.reduced(7, 2));
    }
    auto footer = footerArea.reduced(14, 10);
    footer.removeFromTop(22);
    master.setBounds(footer.removeFromLeft(juce::jmin(330, footer.getWidth() / 3)));
    footer.removeFromLeft(40);
    room.setBounds(footer.removeFromLeft(juce::jmin(260, footer.getWidth() / 2)));
}

void RaBubanjEditor::paint(juce::Graphics& g)
{
    g.fillAll(background);
    g.setColour(accent);
    g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    g.drawText("V O Y A G E R S Q U A R E  /  INSTRUMENTS", 30, 23, 600, 23, juce::Justification::centredLeft);
    g.setColour(ink);
    g.setFont(juce::FontOptions(34.0f, juce::Font::bold));
    g.drawText("RaBubanj", 28, 49, 540, 48, juce::Justification::centredLeft);
    g.setColour(muted);
    g.setFont(juce::FontOptions(14.0f));
    g.drawText("STUDIO KIT  /  08 DRUMS  /  v0.1", getWidth() - 370, 56, 338, 26, juce::Justification::centredRight);
    g.setFont(juce::FontOptions(12.0f));
    g.drawText("KIT  /  CLICK TO AUDITION", padArea.getX(), padArea.getY() - 19, 320, 18, juce::Justification::centredLeft);
    g.drawText("MIXER  /  LEVEL + PAN", mixerArea.getX(), mixerArea.getY() - 23, 320, 18, juce::Justification::centredLeft);
    const int width = padArea.getWidth() / rabubanj::drumCount;
    for (int d = 0; d < rabubanj::drumCount; ++d)
    {
        const auto& channel = channels[static_cast<size_t>(d)];
        auto rect = juce::Rectangle<float>(static_cast<float>(mixerArea.getX() + d * width + 4),
            static_cast<float>(mixerArea.getY()), static_cast<float>(width - 8), static_cast<float>(mixerArea.getHeight()));
        g.setColour(panel);
        g.fillRoundedRectangle(rect, 8);
        g.setColour(muted);
        g.drawText(rabubanj::names[static_cast<size_t>(d)], rect.toNearestInt().removeFromTop(32), juce::Justification::centred);
        g.setColour(accent.withAlpha(.15f + channel.flash * .85f));
        g.fillRoundedRectangle(static_cast<float>(padArea.getX() + d * width + 12), static_cast<float>(padArea.getBottom() - 5), static_cast<float>(width - 24), 3, 1);
    }
    g.setColour(panel);
    g.fillRoundedRectangle(footerArea.toFloat(), 8);
    g.setColour(muted);
    g.drawText("MASTER", master.getX(), footerArea.getY() + 9, 150, 21, juce::Justification::centredLeft);
    g.drawText("ROOM", room.getX(), footerArea.getY() + 9, 150, 21, juce::Justification::centredLeft);
    const auto meter = juce::Rectangle<float>(static_cast<float>(getWidth() - 228), static_cast<float>(footerArea.getY() + 39), 174, 9);
    g.setColour(background);
    g.fillRoundedRectangle(meter, 3);
    g.setColour(accent);
    g.fillRoundedRectangle(meter.withWidth(meter.getWidth() * juce::jlimit(0.0f, 1.0f, processor.getPeak())), 3);
    g.setColour(muted);
    g.drawText("STEREO OUT", getWidth() - 228, footerArea.getY() + 10, 174, 20, juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(11.0f));
    g.drawText("GM MIDI  /  VELOCITY LAYERS + ROUND ROBIN  /  GENERATED STARTER KIT", 28, getHeight() - 24, getWidth() - 56, 18, juce::Justification::centredLeft);
}

void RaBubanjEditor::timerCallback()
{
    for (int d = 0; d < rabubanj::drumCount; ++d)
    {
        auto& channel = channels[static_cast<size_t>(d)];
        const auto count = processor.getHitCount(d);
        channel.flash = count != channel.lastHit ? 1.0f : channel.flash * .78f;
        channel.lastHit = count;
    }
    repaint();
}
