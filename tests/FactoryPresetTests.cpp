#include "DrumEngine.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>

static void require(bool condition, const char* message)
{
    if (!condition) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); }
}
static std::vector<float> render(rabubanj::DrumEngine& engine, int drum, int preset, double rate, float velocity)
{
    engine.reset(); engine.setPreset(drum, preset);
    const auto count = static_cast<int>(rate*4);
    std::vector<float> left(static_cast<size_t>(count)), right(left.size());
    rabubanj::Mix mix; mix.pan.fill(0);
    engine.trigger(rabubanj::notes[static_cast<size_t>(drum)], velocity);
    // Different host-sized slices also exercise sample-rate conversion continuity.
    for (int offset = 0; offset < count;)
    {
        const int block = std::min(193, count-offset);
        engine.render(left.data()+offset, right.data()+offset, block, mix); offset += block;
    }
    require(engine.activeVoiceCount() == 0, "factory one-shots must finish within four seconds");
    for (auto& sample : left) sample *= 1.41421356237f;
    return left;
}
static double energy(const std::vector<float>& data)
{ return std::inner_product(data.begin(), data.end(), data.begin(), 0.0); }
static void writeWav(const std::filesystem::path& path, const std::vector<float>& data)
{
    size_t end = data.size(); while (end > 1 && std::abs(data[end-1]) < .00001f) --end;
    end = std::min(data.size(), end+320);
    std::ofstream stream(path, std::ios::binary); require(static_cast<bool>(stream), "preview output must open");
    auto u16 = [&](uint16_t v) { stream.put(static_cast<char>(v & 255)); stream.put(static_cast<char>(v >> 8)); };
    auto u32 = [&](uint32_t v) { u16(static_cast<uint16_t>(v)); u16(static_cast<uint16_t>(v >> 16)); };
    stream.write("RIFF", 4); u32(static_cast<uint32_t>(36+end*2)); stream.write("WAVEfmt ", 8); u32(16); u16(1); u16(1);
    u32(32000); u32(64000); u16(2); u16(16); stream.write("data", 4); u32(static_cast<uint32_t>(end*2));
    for (size_t i = 0; i < end; ++i) u16(static_cast<uint16_t>(static_cast<int16_t>(std::lround(std::clamp(data[i], -1.0f, 1.0f)*32767))));
    require(static_cast<bool>(stream), "preview WAV must be written");
}
int main(int argc, char** argv)
{
    rabubanj::DrumEngine engine;
    if (argc > 1)
    {
        const std::filesystem::path directory(argv[1]); std::filesystem::create_directories(directory);
        engine.prepare(32000);
        for (int drum = 0; drum < 8; ++drum) for (int preset = 0; preset < rabubanj::factoryPresetCount; ++preset)
        {
            std::string name = rabubanj::names[static_cast<size_t>(drum)]; std::replace(name.begin(), name.end(), ' ', '-');
            const auto number = std::string(preset < 9 ? "0" : "")+std::to_string(preset+1);
            const auto filename = name+"-"+number+"-"+rabubanj::factoryPresets[static_cast<size_t>(drum)][static_cast<size_t>(preset)].name+".wav";
            writeWav(directory/filename, render(engine, drum, preset, 32000, .92f));
        }
        std::cout << "Rendered 160 original mono factory previews (32 kHz, 16-bit PCM).\n"; return 0;
    }
    for (double rate : { 44100.0, 48000.0, 96000.0 })
    {
        engine.prepare(rate);
        for (int drum = 0; drum < 8; ++drum)
        {
            std::array<std::vector<float>, rabubanj::factoryPresetCount> sounds;
            for (int preset = 0; preset < rabubanj::factoryPresetCount; ++preset)
            {
                sounds[static_cast<size_t>(preset)] = render(engine, drum, preset, rate, .9f);
                require(energy(sounds[static_cast<size_t>(preset)]) > .005, "every factory preset must be audible");
                for (float value : sounds[static_cast<size_t>(preset)]) require(std::isfinite(value) && std::abs(value) < 1.5f, "factory output must be finite and bounded");
                const auto quiet = render(engine, drum, preset, rate, .2f);
                require(energy(sounds[static_cast<size_t>(preset)]) > energy(quiet)*6, "every preset must retain velocity dynamics");
                for (int previous = 0; previous < preset; ++previous)
                {
                    double difference = 0;
                    for (size_t i = 0; i < sounds[0].size(); ++i)
                    {
                        const auto d = sounds[static_cast<size_t>(preset)][i]-sounds[static_cast<size_t>(previous)][i]; difference += d*d;
                    }
                    require(difference > .03, "factory choices must have distinct rendered audio");
                }
            }
        }
        engine.reset(); engine.setPreset(3, 5); engine.setPreset(2, 8);
        engine.trigger(39, .9f); engine.trigger(38, .9f);
        std::vector<float> left(static_cast<size_t>(rate*.5)), right(left.size()); rabubanj::Mix mix;
        engine.render(left.data(), right.data(), static_cast<int>(left.size()), mix);
        require(engine.activeVoiceCount() == 0, "closed factory hats must choke open factory hats");
        rabubanj::DrumEngine reference;
        reference.prepare(rate); reference.setPreset(6, 1); reference.trigger(42, .9f);
        engine.reset(); engine.setPreset(6, 1); engine.trigger(42, .9f); engine.setPreset(6, 9);
        std::vector<float> changed(4096), changedRight(4096), unchanged(4096), unchangedRight(4096);
        engine.render(changed.data(), changedRight.data(), 4096, mix);
        reference.render(unchanged.data(), unchangedRight.data(), 4096, mix);
        require(changed == unchanged && changedRight == unchangedRight, "preset selection must not alter a playing tail");
        engine.setPreset(0, -100); require(engine.getPreset(0) == 0, "low invalid preset must clamp");
        engine.setPreset(0, 100); require(engine.getPreset(0) == 19, "high invalid preset must clamp");
    }
    require(rabubanj::FactoryBank::get().sampleBytes() < 256u*1024u*1024u, "shared factory sample storage must fit its budget");
    std::cout << "All 160 presets: audible, distinct, finite, velocity-sensitive, bounded tails, sample rates, choke and live selection passed.\n";
    std::cout << "Shared factory PCM bytes: " << rabubanj::FactoryBank::get().sampleBytes() << '\n';
}
