#include "ParametricEq.h"
#include <cstdlib>
#include <iostream>
#include <vector>
static void require(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
int main()
{
    using namespace rabubanj;
    for (const double rate : { 44100.0, 48000.0, 96000.0 })
    {
        EqBand band { 1000, 12, 1, 0, true };
        const auto db = 20*std::log10(eqCoefficients(band, rate).magnitude(1000, rate));
        require(std::abs(db - 12) < .0001, "bell centre must match requested gain");
        band.type = 1;
        require(eqCoefficients(band, rate).magnitude(1, rate) > 3.9 && eqCoefficients(band, rate).magnitude(rate*.49, rate) < 1.001,
            "low shelf must boost lows and leave highs alone");
        band.type = 2;
        require(eqCoefficients(band, rate).magnitude(1, rate) < 1.001 && eqCoefficients(band, rate).magnitude(rate*.49, rate) > 3.9,
            "high shelf must boost highs and leave lows alone");
        band.type = 3;
        require(eqCoefficients(band, rate).magnitude(50, rate) < .01 && eqCoefficients(band, rate).magnitude(10000, rate) > .99,
            "low cut must reject lows");
        band.type = 4;
        require(eqCoefficients(band, rate).magnitude(10000, rate) < .01 && eqCoefficients(band, rate).magnitude(50, rate) > .99,
            "high cut must reject highs");
        band.type = 5;
        require(eqCoefficients(band, rate).magnitude(1000, rate) < 1e-10, "notch must reject its centre");

        EqSettings settings;
        settings.bands[0] = { 1000, 12, 1, 0, true };
        ParametricEq eq; eq.prepare(rate); eq.setSettings(settings);
        double inputEnergy = 0, outputEnergy = 0;
        for (int i = 0; i < 24000; ++i)
        {
            const auto input = static_cast<float>(.05*std::sin(6.283185307179586*1000*i/rate));
            const auto output = eq.processMono(input);
            if (i > 16000) { inputEnergy += input*input; outputEnergy += output*output; }
        }
        require(std::abs(10*std::log10(outputEnergy/inputEnergy)-12) < .02, "actual audio must match response graph");
        settings.bypass = true; eq.prepare(rate); eq.setSettings(settings);
        for (int i = 0; i < 100; ++i)
        {
            const float input = static_cast<float>(std::sin(static_cast<double>(i)));
            require(std::abs(eq.processMono(input)-input) < 1e-7f, "EQ bypass must pass audio unchanged");
        }
        settings.bypass = false; settings.bands[0].enabled = false; eq.prepare(rate); eq.setSettings(settings);
        require(std::abs(eq.processMono(.25f)-.25f) < 1e-7f, "disabled band must not affect audio");

        for (int type = 0; type < 6; ++type)
            for (float frequency : { 20.0f, 20000.0f })
                for (float gain : { -24.0f, 24.0f })
                {
                    settings = {}; settings.bands[0] = { frequency, gain, 18, type, true };
                    eq.prepare(rate); eq.setSettings(settings);
                    for (int i = 0; i < 12000; ++i)
                    {
                        float left = i == 0 ? .1f : 0, right = left;
                        eq.processStereo(left, right);
                        require(std::isfinite(left) && std::isfinite(right) && std::abs(left) < 100,
                            "extreme frequency / Q / gain must remain stable");
                    }
                }
        settings = {}; settings.bands[2] = { 700, -12, 3, 0, true };
        ParametricEq a, b; a.prepare(rate); b.prepare(rate); a.setSettings(settings); b.setSettings(settings);
        for (int i = 0; i < 5000; ++i)
        {
            if (i % 13 == 0) a.setSettings(settings);
            if (i % 512 == 0) b.setSettings(settings);
            const float input = static_cast<float>(std::sin(i*.2)*.1);
            require(std::abs(a.processMono(input)-b.processMono(input)) < 1e-7f, "host block boundaries must not change EQ output");
        }
        eq.prepare(rate); settings = {}; eq.setSettings(settings);
        for (int i = 0; i < 24000; ++i)
        {
            if (i % 100 == 0)
            {
                auto& node = settings.bands[0]; node.frequency = i % 200 == 0 ? 20.0f : 20000.0f;
                node.gain = i % 200 == 0 ? -24.0f : 24.0f; node.q = 18; eq.setSettings(settings);
            }
            require(std::isfinite(eq.processMono(.01f)), "fast EQ automation must remain finite");
        }
    }
    std::cout << "EQ shapes, audio response, bypass, stability, automation and block continuity passed.\n";
}
