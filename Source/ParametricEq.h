#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <string>

namespace rabubanj
{
constexpr int eqBandCount = 6;
constexpr int eqBusCount = 10; // 8 drums, reverb return, stereo master
constexpr int reverbBus = 8, masterBus = 9;
inline constexpr std::array<float, eqBandCount> eqFrequencies { 60, 180, 650, 2500, 7000, 14000 };
inline constexpr std::array<const char*, 6> eqTypeNames { "Bell", "Low shelf", "High shelf", "Low cut", "High cut", "Notch" };
inline std::string eqParameterId(int bus, int band, const char* field)
{
    return "eq" + std::to_string(bus) + "b" + std::to_string(band) + field;
}
struct EqBand
{
    float frequency = 1000, gain = 0, q = .7071f;
    int type = 0;
    bool enabled = true;
};
struct EqSettings
{
    std::array<EqBand, eqBandCount> bands;
    bool bypass = false;
    EqSettings() { for (size_t i = 0; i < bands.size(); ++i) bands[i].frequency = eqFrequencies[i]; }
};
struct Biquad
{
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    double magnitude(double frequency, double rate) const noexcept
    {
        const auto z = std::polar(1.0, -6.283185307179586 * frequency / rate);
        return std::abs((b0 + b1 * z + b2 * z * z) / (1.0 + a1 * z + a2 * z * z));
    }
};
inline Biquad eqCoefficients(const EqBand& band, double rate) noexcept
{
    rate = std::max(100.0, rate);
    const auto frequency = std::clamp(static_cast<double>(band.frequency), 10.0, rate * .475);
    const auto w = 6.283185307179586 * frequency / rate;
    const auto c = std::cos(w), s = std::sin(w);
    const auto q = std::clamp(static_cast<double>(band.q), .15, 18.0);
    const auto a = std::pow(10.0, std::clamp(static_cast<double>(band.gain), -24.0, 24.0) / 40.0);
    const auto alpha = s / (2.0 * q), beta = 2.0 * std::sqrt(a) * alpha;
    double b0 = 1, b1 = 0, b2 = 0, a0 = 1, a1 = 0, a2 = 0;
    switch (band.type)
    {
        case 1:
            b0 = a * ((a+1) - (a-1)*c + beta); b1 = 2*a*((a-1) - (a+1)*c); b2 = a*((a+1) - (a-1)*c - beta);
            a0 = (a+1) + (a-1)*c + beta; a1 = -2*((a-1) + (a+1)*c); a2 = (a+1) + (a-1)*c - beta; break;
        case 2:
            b0 = a*((a+1) + (a-1)*c + beta); b1 = -2*a*((a-1) + (a+1)*c); b2 = a*((a+1) + (a-1)*c - beta);
            a0 = (a+1) - (a-1)*c + beta; a1 = 2*((a-1) - (a+1)*c); a2 = (a+1) - (a-1)*c - beta; break;
        case 3:
            b0 = (1+c)*.5; b1 = -(1+c); b2 = b0; a0 = 1+alpha; a1 = -2*c; a2 = 1-alpha; break;
        case 4:
            b0 = (1-c)*.5; b1 = 1-c; b2 = b0; a0 = 1+alpha; a1 = -2*c; a2 = 1-alpha; break;
        case 5:
            b0 = 1; b1 = -2*c; b2 = 1; a0 = 1+alpha; a1 = -2*c; a2 = 1-alpha; break;
        default:
            b0 = 1+alpha*a; b1 = -2*c; b2 = 1-alpha*a; a0 = 1+alpha/a; a1 = -2*c; a2 = 1-alpha/a; break;
    }
    return { b0/a0, b1/a0, b2/a0, a1/a0, a2/a0 };
}
inline double eqMagnitude(const EqSettings& settings, double frequency, double rate) noexcept
{
    double result = 1;
    if (!settings.bypass)
        for (const auto& band : settings.bands)
            if (band.enabled) result *= eqCoefficients(band, rate).magnitude(frequency, rate);
    return result;
}

// Fixed storage, double precision filter states. Parameters are smoothed, then
// coefficients recomputed every 32 samples, independently of host block size.
class ParametricEq
{
public:
    void prepare(double rate) noexcept
    {
        sampleRate = std::max(100.0, rate);
        controlAlpha = 1.0 - std::exp(-32.0 / (sampleRate * .012));
        wetAlpha = 1.0 - std::exp(-1.0 / (sampleRate * .005));
        reset();
        initialised = false;
    }
    void reset() noexcept { states = {}; tick = 0; }
    void setSettings(const EqSettings& next) noexcept
    {
        target = next;
        if (!initialised)
        {
            current = target;
            for (size_t i = 0; i < wet.size(); ++i) wet[i] = target.bypass || !target.bands[i].enabled ? 0.0 : 1.0;
            initialised = true;
            tick = 0;
        }
    }
    float processMono(float input) noexcept { float unused = 0; process(input, unused, false); return input; }
    void processStereo(float& left, float& right) noexcept { process(left, right, true); }
private:
    struct Memory { double z1 = 0, z2 = 0; };
    std::array<std::array<Memory, 2>, eqBandCount> states {};
    std::array<Biquad, eqBandCount> coefficients {};
    std::array<double, eqBandCount> wet {};
    EqSettings current, target;
    double sampleRate = 44100, controlAlpha = .05, wetAlpha = .005;
    int tick = 0;
    bool initialised = false;
    void process(float& left, float& right, bool stereo) noexcept
    {
        if (tick-- <= 0)
        {
            tick = 31;
            for (size_t i = 0; i < coefficients.size(); ++i)
            {
                auto& b = current.bands[i]; const auto& t = target.bands[i];
                b.frequency += static_cast<float>((t.frequency - b.frequency) * controlAlpha);
                b.gain += static_cast<float>((t.gain - b.gain) * controlAlpha);
                b.q += static_cast<float>((t.q - b.q) * controlAlpha);
                if (b.type != t.type) { b.type = t.type; states[i] = {}; wet[i] = 0; }
                coefficients[i] = eqCoefficients(b, sampleRate);
            }
        }
        double l = left, r = right;
        for (size_t i = 0; i < coefficients.size(); ++i)
        {
            const double wanted = target.bypass || !target.bands[i].enabled ? 0.0 : 1.0;
            wet[i] += (wanted - wet[i]) * wetAlpha;
            const auto& c = coefficients[i];
            auto apply = [&](double input, Memory& state)
            {
                const auto output = c.b0 * input + state.z1;
                state.z1 = c.b1 * input - c.a1 * output + state.z2;
                state.z2 = c.b2 * input - c.a2 * output;
                return input + (output - input) * wet[i];
            };
            l = apply(l, states[i][0]);
            if (stereo) r = apply(r, states[i][1]);
        }
        left = static_cast<float>(l);
        if (stereo) right = static_cast<float>(r);
    }
};
}
