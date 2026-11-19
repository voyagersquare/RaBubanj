#pragma once
#include <algorithm>
#include <cmath>
namespace rabubanj
{
// Stereo-linked, one-knob soft-knee compression and antialiased soft saturation.
// Coefficients and all state have fixed storage; no allocation or locks during playback.
class ChannelColour
{
public:
    void prepare(double rate) noexcept
    {
        sampleRate = std::max(8000.0, rate); smoothing = 1-std::exp(-1/(sampleRate*.005));
        attack = std::exp(-1/(sampleRate*.010)); reset();
        drive = targetDrive; compression = targetCompression;
    }
    void reset() noexcept { previousLeft = previousRight = reduction = 0; }
    void set(float driveAmount, float compressionAmount) noexcept
    { targetDrive = std::clamp(static_cast<double>(driveAmount), 0.0, 1.0); targetCompression = std::clamp(static_cast<double>(compressionAmount), 0.0, 1.0); }
    double gainReductionDb() const noexcept { return reduction*compression; }
    void process(float& left, float& right) noexcept
    {
        drive += (targetDrive-drive)*smoothing; compression += (targetCompression-compression)*smoothing;
        const auto l = static_cast<double>(left), r = static_cast<double>(right);
        if (drive > .000001)
        {
            const auto strength = 1+7*drive, normalise = 1/std::tanh(strength);
            auto saturate = [&](double input, double previous)
            {
                auto primitive = [&](double x)
                { const auto a = std::abs(strength*x); return (a+std::log1p(std::exp(-2*a))-.6931471805599453)/strength; };
                const auto delta = input-previous;
                // First-order antiderivative antialiasing avoids a harsh folded high end.
                return (std::abs(delta) > .00001 ? (primitive(input)-primitive(previous))/delta
                    : std::tanh(strength*(input+previous)*.5))*normalise;
            };
            left = static_cast<float>(l+drive*(saturate(l, previousLeft)-l));
            right = static_cast<float>(r+drive*(saturate(r, previousRight)-r));
        }
        previousLeft = l; previousRight = r;
        const auto level = std::max(std::abs(static_cast<double>(left)), std::abs(static_cast<double>(right)));
        const auto over = 20*std::log10(std::max(1e-9, level))+18; // -18 dBFS threshold, 6 dB knee, 3:1 ratio.
        const auto desired = over < -3 ? 0.0 : over > 3 ? over*(2.0/3.0) : (over+3)*(over+3)/18;
        // Opto-inspired programme-dependent release: 150 ms for small reductions, up to 350 ms.
        const auto release = std::exp(-1/(sampleRate*(.150+.200*std::clamp(reduction/12, 0.0, 1.0))));
        reduction = desired+(reduction-desired)*(desired > reduction ? attack : release);
        if (compression > .000001)
        {
            const auto gain = 1+compression*(std::pow(10.0, -reduction/20)-1);
            left *= static_cast<float>(gain); right *= static_cast<float>(gain);
        }
    }
private:
    double sampleRate = 44100, smoothing = .0045, attack = .9977;
    double drive = 0, compression = 0, targetDrive = 0, targetCompression = 0;
    double previousLeft = 0, previousRight = 0, reduction = 0;
};
}
