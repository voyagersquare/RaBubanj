#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>

namespace rabubanj
{
// One audio producer and one GUI consumer per bus. FFTs run exclusively in the GUI.
class SpectrumTap
{
public:
    void push(const float* l, const float* r, int count) noexcept
    {
        int start1, size1, start2, size2; fifo.prepareToWrite(count, start1, size1, start2, size2);
        for (int i = 0; i < size1; ++i) { left[static_cast<size_t>(start1+i)] = l[i]; right[static_cast<size_t>(start1+i)] = r[i]; }
        for (int i = 0; i < size2; ++i) { left[static_cast<size_t>(start2+i)] = l[size1+i]; right[static_cast<size_t>(start2+i)] = r[size1+i]; }
        fifo.finishedWrite(size1+size2);
    }
    int read(float* l, float* r, int maximum) noexcept
    {
        int start1, size1, start2, size2; fifo.prepareToRead(maximum, start1, size1, start2, size2);
        for (int i = 0; i < size1; ++i) { l[i] = left[static_cast<size_t>(start1+i)]; r[i] = right[static_cast<size_t>(start1+i)]; }
        for (int i = 0; i < size2; ++i) { l[size1+i] = left[static_cast<size_t>(start2+i)]; r[size1+i] = right[static_cast<size_t>(start2+i)]; }
        fifo.finishedRead(size1+size2); return size1+size2;
    }
    void discard() noexcept { fifo.finishedRead(fifo.getNumReady()); }
private:
    juce::AbstractFifo fifo { 8192 };
    std::array<float, 8192> left {}, right {};
};
class SpectrumAnalyser
{
public:
    static constexpr int order = 12, size = 1 << order;
    SpectrumAnalyser()
    {
        for (int i = 0; i < size; ++i) { window[static_cast<size_t>(i)] = .5f-.5f*std::cos(juce::MathConstants<float>::twoPi*static_cast<float>(i)/static_cast<float>(size-1)); windowSum += window[static_cast<size_t>(i)]; }
        clear();
    }
    void clear() noexcept { historyLeft.fill(0); historyRight.fill(0); bins.fill(-100); writePosition = pending = 0; }
    void update(SpectrumTap& tap)
    {
        int read;
        while ((read = tap.read(scratchLeft.data(), scratchRight.data(), static_cast<int>(scratchLeft.size()))) > 0)
            for (int i = 0; i < read; ++i)
            {
                historyLeft[static_cast<size_t>(writePosition)] = scratchLeft[static_cast<size_t>(i)];
                historyRight[static_cast<size_t>(writePosition)] = scratchRight[static_cast<size_t>(i)];
                writePosition = (writePosition+1)%size; ++pending;
            }
        if (pending >= 1024)
        {
            for (int i = 0; i < size; ++i)
            {
                const auto index = static_cast<size_t>((writePosition+i)%size);
                fftLeft[static_cast<size_t>(i)] = historyLeft[index]*window[static_cast<size_t>(i)];
                fftRight[static_cast<size_t>(i)] = historyRight[index]*window[static_cast<size_t>(i)];
            }
            std::fill(fftLeft.begin()+size, fftLeft.end(), 0); std::fill(fftRight.begin()+size, fftRight.end(), 0);
            fft.performFrequencyOnlyForwardTransform(fftLeft.data()); fft.performFrequencyOnlyForwardTransform(fftRight.data());
            for (size_t i = 0; i < bins.size(); ++i)
            {
                const auto value = juce::Decibels::gainToDecibels(std::max(fftLeft[i], fftRight[i])*2/windowSum, -100.0f);
                bins[i] = value > bins[i] ? value : bins[i]*.78f+value*.22f;
            }
            pending = 0;
        }
        else for (auto& bin : bins) bin = std::max(-100.0f, bin-2.5f);
    }
    float magnitudeAt(float frequency, double sampleRate) const noexcept
    {
        const auto position = static_cast<float>(frequency*size/sampleRate);
        if (position < 0 || position >= size*.5f) return -100;
        const auto index = static_cast<size_t>(position); const auto fraction = position-static_cast<float>(index);
        return bins[index]+(bins[index+1]-bins[index])*fraction;
    }
    const std::array<float, size/2+1>& getBins() const noexcept { return bins; }
private:
    juce::dsp::FFT fft { order };
    std::array<float, size> historyLeft {}, historyRight {}, window {};
    std::array<float, size*2> fftLeft {}, fftRight {};
    std::array<float, 1024> scratchLeft {}, scratchRight {};
    std::array<float, size/2+1> bins {};
    float windowSum = 0;
    int writePosition = 0, pending = 0;
};
}
