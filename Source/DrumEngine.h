#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
#include "FactoryPresets.h"

namespace rabubanj
{
constexpr int drumCount = 8;
inline constexpr std::array<const char*, drumCount> names {
    "Kick", "Snare", "Closed Hat", "Open Hat", "Low Tom", "High Tom", "Crash", "Ride" };
inline constexpr std::array<int, drumCount> notes { 36, 38, 42, 46, 45, 48, 49, 51 };
inline int drumForNote(int note) noexcept
{
    switch (note)
    {
        case 35: case 36: return 0;
        case 37: case 38: case 40: return 1;
        case 42: case 44: return 2;
        case 46: return 3;
        case 41: case 43: case 45: return 4;
        case 47: case 48: case 50: return 5;
        case 49: case 52: case 55: case 57: return 6;
        case 51: case 53: case 59: return 7;
        default: return -1;
    }
}

struct Mix
{
    std::array<float, drumCount> gain { 1, 1, 1, 1, 1, 1, 1, 1 };
    std::array<float, drumCount> pan { 0, 0, -.3f, -.3f, .3f, -.3f, -.5f, .5f };
    std::array<bool, drumCount> mute {};
};

// Fixed voice pool; sample memory is built only in prepare(), never in render().
class DrumEngine
{
public:
    void prepare(double rate)
    {
        sampleRate = rate > 0.0 ? rate : 44100.0;
        reset();
        factory = &FactoryBank::get();
        for (int drum = 0; drum < drumCount; ++drum)
            for (int layer = 0; layer < 3; ++layer)
                for (int rr = 0; rr < 3; ++rr)
                    generate(drum, layer, rr);
    }

    void reset() noexcept
    {
        voices = {};
        counters = {};
        serial = 0;
    }

    void setPreset(int drum, int preset) noexcept
    {
        if (drum >= 0 && drum < drumCount) presetIndices[static_cast<size_t>(drum)] = std::clamp(preset, 0, factoryPresetCount-1);
    }
    int getPreset(int drum) const noexcept { return presetIndices[static_cast<size_t>(drum)]; }

    void trigger(int note, float velocity) noexcept
    {
        const int drum = drumForNote(note);
        if (drum < 0 || velocity <= 0.0f) return;
        velocity = std::clamp(velocity, 0.0f, 1.0f);
        if (drum == 2 || drum == 3)
            for (auto& voice : voices)
                if (voice.active && voice.drum == 3)
                    voice.choke = std::max(1, static_cast<int>(sampleRate * .005));

        auto* selected = &voices.front();
        for (auto& voice : voices)
        {
            if (!voice.active) { selected = &voice; break; }
            if (voice.age < selected->age) selected = &voice;
        }
        const auto layer = std::min(2, static_cast<int>(velocity * 3));
        const auto rr = counters[static_cast<size_t>(drum)]++ % 3;
        const auto preset = presetIndices[static_cast<size_t>(drum)];
        *selected = {};
        selected->active = true; selected->drum = drum; selected->velocity = velocity; selected->age = ++serial;
        selected->sample = preset == 0 || factory == nullptr ? &bank[static_cast<size_t>(drum)][static_cast<size_t>(layer)][static_cast<size_t>(rr)]
            : &factory->sample(drum, preset, layer, rr);
        selected->increment = preset == 0 || factory == nullptr ? 1.0 : factorySampleRate/sampleRate;
    }

    void render(float* left, float* right, int count, const Mix& mix) noexcept
    {
        std::array<float, drumCount> leftGain {}, rightGain {};
        for (int d = 0; d < drumCount; ++d)
        {
            const auto i = static_cast<size_t>(d);
            const float angle = (std::clamp(mix.pan[i], -1.0f, 1.0f) + 1.0f) * pi * .25f;
            const auto gain = mix.mute[i] ? 0.0f : mix.gain[i];
            leftGain[i] = gain * std::cos(angle);
            rightGain[i] = gain * std::sin(angle);
        }
        renderVoices(count, [&](size_t drum, int sample, float value)
        {
            left[sample] += value * leftGain[drum];
            right[sample] += value * rightGain[drum];
        });
    }

    void renderStems(const std::array<float*, drumCount>& outputs, int count) noexcept
    {
        renderVoices(count, [&](size_t drum, int sample, float value) { outputs[drum][sample] += value; });
    }

    int activeVoiceCount() const noexcept
    {
        return static_cast<int>(std::count_if(voices.begin(), voices.end(), [](const Voice& v) { return v.active; }));
    }

private:
    static constexpr float pi = 3.14159265358979323846f;
    struct Voice
    {
        bool active = false;
        int drum = 0;
        const std::vector<float>* sample = nullptr;
        double position = 0, increment = 1;
        int choke = 0;
        float velocity = 0;
        uint64_t age = 0;
    };
    std::array<Voice, 64> voices {};
    std::array<int, drumCount> counters {};
    std::array<int, drumCount> presetIndices {};
    const FactoryBank* factory = nullptr;
    std::array<std::array<std::array<std::vector<float>, 3>, 3>, drumCount> bank;
    double sampleRate = 44100;
    uint64_t serial = 0;

    template <typename Sink> void renderVoices(int count, Sink&& sink) noexcept
    {
        for (auto& voice : voices)
        {
            if (!voice.active) continue;
            const auto drum = static_cast<size_t>(voice.drum);
            const auto& sample = *voice.sample;
            for (int s = 0; s < count; ++s)
            {
                const auto position = static_cast<size_t>(voice.position);
                if (position >= sample.size()) { voice.active = false; break; }
                const auto next = position+1 < sample.size() ? sample[position+1] : 0.0f;
                const auto fraction = static_cast<float>(voice.position-static_cast<double>(position));
                float value = (sample[position]+(next-sample[position])*fraction)*voice.velocity;
                voice.position += voice.increment;
                if (voice.choke > 0)
                {
                    value *= static_cast<float>(voice.choke) / static_cast<float>(std::max(1, static_cast<int>(sampleRate * .005)));
                    if (--voice.choke == 0) voice.active = false;
                }
                sink(drum, s, value);
                if (!voice.active) break;
            }
        }
    }

    void generate(int drum, int layer, int rr)
    {
        constexpr std::array<float, drumCount> duration { .9f, .65f, .18f, 1.3f, 1.1f, .85f, 2.8f, 2.0f };
        auto& sample = bank[static_cast<size_t>(drum)][static_cast<size_t>(layer)][static_cast<size_t>(rr)];
        sample.resize(static_cast<size_t>(sampleRate * duration[static_cast<size_t>(drum)]));
        uint32_t rng = 1729u + static_cast<uint32_t>(drum * 127 + layer * 31 + rr * 337);
        const float variant = 1.0f + static_cast<float>(rr - 1) * .012f;
        const float brightness = .65f + .175f * static_cast<float>(layer);
        double phase = 0;
        float lowNoise = 0;
        const float noiseCoeff = 1.0f - std::exp(-2.0f * pi * 2400.0f / static_cast<float>(sampleRate));
        for (size_t i = 0; i < sample.size(); ++i)
        {
            const float t = static_cast<float>(static_cast<double>(i) / sampleRate);
            rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
            const float noise = static_cast<float>(rng & 0xffffu) / 32767.5f - 1.0f;
            lowNoise += noiseCoeff * (noise - lowNoise);
            const float highNoise = noise - lowNoise;
            float value = 0;
            if (drum == 0)
            {
                phase += 2.0 * pi * (48.0 + 115.0 * std::exp(-t * 50.0f)) * variant / sampleRate;
                value = .85f * static_cast<float>(std::sin(phase)) * std::exp(-t * 9.0f)
                      + noise * .22f * brightness * std::exp(-t * 160.0f);
            }
            else if (drum == 1)
            {
                phase += 2.0 * pi * 185.0 * variant / sampleRate;
                value = .45f * static_cast<float>(std::sin(phase)) * std::exp(-t * 24.0f)
                      + highNoise * .62f * brightness * std::exp(-t * 16.0f);
            }
            else if (drum == 4 || drum == 5)
            {
                const float freq = drum == 4 ? 95.0f : 145.0f;
                phase += 2.0 * pi * freq * (1.0 + .6 * std::exp(-t * 35.0f)) * variant / sampleRate;
                value = .75f * static_cast<float>(std::sin(phase)) * std::exp(-t * 7.5f)
                      + noise * .15f * brightness * std::exp(-t * 80.0f);
            }
            else
            {
                float metal = 0;
                for (float frequency : { 3167.0f, 4211.0f, 5873.0f, 7451.0f })
                    metal += std::sin(2.0f * pi * frequency * variant * t) * .12f;
                const float decay = drum == 2 ? 42.0f : drum == 3 ? 5.0f : drum == 6 ? 2.6f : 3.6f;
                value = (highNoise * .48f + metal * .32f) * brightness * std::exp(-t * decay);
                if (drum == 7)
                    value += .17f * std::sin(2.0f * pi * 1560.0f * variant * t) * std::exp(-t * 6.0f);
            }
            const float attack = std::min(1.0f, t * 2000.0f);
            const float tail = std::min(1.0f, static_cast<float>(sample.size() - i - 1) / static_cast<float>(sampleRate * .01));
            sample[i] = value * attack * tail;
        }
    }
};
}
