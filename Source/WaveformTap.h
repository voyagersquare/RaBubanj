#pragma once
#include <juce_core/juce_core.h>
#include <array>
#include <atomic>
#include <algorithm>
#include <cmath>

namespace rabubanj
{
// Dedicated SPSC scope feed, independent of the EQ FFT consumers. All storage is bounded.
class WaveformTap
{
public:
    struct Frame { float low = 0, high = 0; uint32_t token = 0; };
    void selectPad(int pad) noexcept
    {
        const auto value = static_cast<uint32_t>(pad >= 0 && pad < 8 ? pad+1 : 0);
        const auto old = request.load();
        if ((old & 15u) != value) request.store(((old+16u) & ~15u) | value);
    }
    uint32_t getToken() const noexcept { return request.load(); }
    static int padForToken(uint32_t token) noexcept { return static_cast<int>(token & 15u)-1; }
    void push(const float* left, const float* right, int count, uint32_t token, double rate) noexcept
    {
        if (padForToken(token) < 0) return;
        if (token != audioToken) { audioToken = token; pending = 0; low = high = 0; }
        const auto stride = std::max(1, static_cast<int>(std::round(rate/2000.0)));
        for (int i = 0; i < count; ++i)
        {
            const auto l = std::isfinite(left[i]) ? left[i] : 0.0f, r = std::isfinite(right[i]) ? right[i] : 0.0f;
            low = std::min(low, std::min(l, r)); high = std::max(high, std::max(l, r));
            if (++pending >= stride)
            {
                int start1, size1, start2, size2; fifo.prepareToWrite(1, start1, size1, start2, size2);
                if (size1 > 0) frames[static_cast<size_t>(start1)] = { low, high, token };
                fifo.finishedWrite(size1); pending = 0; low = high = 0;
            }
        }
    }
    int read(Frame* output, int maximum) noexcept
    {
        int start1, size1, start2, size2; fifo.prepareToRead(maximum, start1, size1, start2, size2);
        std::copy_n(frames.data()+start1, size1, output); std::copy_n(frames.data()+start2, size2, output+size1);
        fifo.finishedRead(size1+size2); return size1+size2;
    }
private:
    juce::AbstractFifo fifo { 8192 };
    std::array<Frame, 8192> frames {};
    std::atomic<uint32_t> request { 0 };
    uint32_t audioToken = 0;
    int pending = 0;
    float low = 0, high = 0;
};
}
