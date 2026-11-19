#pragma once
#include "PluginProcessor.h"
#include "UiTheme.h"

class EqPanel final : public juce::Component
{
public:
    EqPanel(RaBubanjProcessor&, int bus, bool isMaster = false);
    ~EqPanel() override;
    void setBus(int);
    int getBus() const noexcept { return busIndex; }
    int getBand() const noexcept { return selectedBand; }
    void selectBand(int);
    void refresh();
    enum ContextCommand { ToggleBand = 1, InvertGain = 2, ShapeBase = 100, GainBase = 200 };
    inline static constexpr std::array<int, 11> gainChoices { -24, -18, -12, -6, -3, 0, 3, 6, 12, 18, 24 };
    juce::PopupMenu makeBandContextMenu(int band) const;
    void applyBandContextAction(int bus, int band, int command);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    class Graph final : public juce::Component, public juce::SettableTooltipClient
    {
    public:
        explicit Graph(EqPanel&);
        ~Graph() override { endGesture(); }
        void resetSpectrum() { spectrum.clear(); }
        void updateSpectrum() { spectrum.update(owner.processor.getSpectrumTap(owner.busIndex)); }
        void cancelGesture() { endGesture(); }
        void paint(juce::Graphics&) override;
        void mouseDown(const juce::MouseEvent&) override;
        void mouseDrag(const juce::MouseEvent&) override;
        void mouseUp(const juce::MouseEvent&) override;
        void mouseDoubleClick(const juce::MouseEvent&) override;
        void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    private:
        EqPanel& owner;
        rabubanj::SpectrumAnalyser spectrum;
        bool dragging = false;
        juce::Rectangle<float> plot() const;
        juce::Point<float> point(const rabubanj::EqBand&) const;
        int nearest(juce::Point<float>) const;
        void moveNode(juce::Point<float>);
        void endGesture();
    };
    juce::RangedAudioParameter* parameter(const char*) const;
    void setBandValue(const char*, float);
    void showBandContextMenu(juce::Component&, juce::Point<int> screenPosition);
    RaBubanjProcessor& processor;
    int busIndex = -1, selectedBand = 0;
    bool masterPanel = false;
    juce::TextButton bypass { "BYPASS" }, reset { "RESET" };
    Graph graph { *this };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::shared_ptr<rabubanj::ui::EqMenuLookAndFeel> menuLook = std::make_shared<rabubanj::ui::EqMenuLookAndFeel>();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EqPanel)
};
