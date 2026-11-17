#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace rabubanj
{
constexpr int factoryPresetCount = 10;
constexpr double factorySampleRate = 32000;
struct DrumPreset
{
    const char* name;
    float frequency, decay, noise, click, harmonic, cutoff, drive;
    int bits;
    float holdRate, sweep;
    bool clap = false;
};
// Slot 01 is the existing native-rate voice; its recipe is retained in DrumEngine.
// Other slots are original analogue-style synthesis recipes, never external samples.
inline constexpr std::array<std::array<DrumPreset, 10>, 8> factoryPresets {{
    {{ {"Thump",48,.11f,.22f,.2f,.1f,9500,1,16,32000,115},
       {"Tape",52,.13f,.10f,.22f,.20f,4700,2.1f,14,22000,105},
       {"Sub",43,.24f,.04f,.09f,.08f,3100,1.5f,15,24000,78},
       {"Knock",64,.09f,.22f,.55f,.42f,6800,2.8f,13,24000,150},
       {"Hollow",57,.16f,.06f,.24f,.62f,4200,1.7f,15,28000,90},
       {"Crunch",55,.12f,.16f,.42f,.38f,5500,4.0f,11,14000,130},
       {"Dust",49,.14f,.14f,.18f,.27f,3300,2.4f,12,12500,100},
       {"Damped",68,.065f,.07f,.35f,.20f,4900,1.9f,14,26000,160},
       {"Round",46,.19f,.03f,.09f,.16f,2600,1.6f,15,24000,65},
       {"Slam",59,.155f,.18f,.44f,.50f,6100,3.3f,12,18000,145} }},
    {{ {"Back",185,.063f,.62f,.1f,.1f,10000,1,16,32000,0},
       {"Crack",224,.055f,.72f,.32f,.34f,8200,2.7f,14,26000,0},
       {"Wood",205,.075f,.32f,.48f,.68f,4300,2.1f,14,24000,0},
       {"Deep",146,.115f,.58f,.14f,.47f,6200,2.3f,14,24000,0},
       {"Clap",178,.075f,.87f,.12f,.25f,7100,2.0f,13,22000,0,true},
       {"Snap",244,.042f,.78f,.30f,.20f,10000,2.8f,14,30000,0},
       {"Grit",190,.085f,.73f,.25f,.46f,4800,4.2f,11,13000,0},
       {"Brush",172,.105f,.91f,.02f,.13f,5000,1.35f,15,26000,0},
       {"Wire",210,.14f,.89f,.18f,.30f,9000,1.9f,14,28000,0},
       {"Ring",232,.13f,.40f,.30f,.82f,6500,2.3f,13,21000,0} }},
    {{ {"Tick",6500,.024f,.48f,.1f,.4f,12000,1,16,32000,0},
       {"Tight",7900,.016f,.40f,.13f,.66f,10800,1.7f,14,30000,0},
       {"Tape",5600,.029f,.65f,.03f,.38f,6600,2.0f,14,22000,0},
       {"Dust",5200,.023f,.81f,.05f,.35f,4700,2.5f,12,12500,0},
       {"Metal",6900,.038f,.28f,.08f,.90f,11000,1.9f,14,28000,0},
       {"Chick",7400,.018f,.61f,.27f,.46f,8500,2.3f,13,23000,0},
       {"Soft",4900,.045f,.76f,.01f,.24f,4100,1.3f,15,26000,0},
       {"Crunch",6300,.027f,.64f,.12f,.72f,6900,3.5f,10,14000,0},
       {"Vinyl",5700,.036f,.86f,.05f,.31f,5500,2.0f,12,16500,0},
       {"Cut",8200,.012f,.48f,.19f,.55f,9600,2.1f,13,28000,0} }},
    {{ {"Sizzle",6500,.20f,.48f,.1f,.4f,12000,1,16,32000,0},
       {"Short",7700,.10f,.48f,.14f,.63f,10200,1.9f,14,30000,0},
       {"Tape",5600,.23f,.66f,.04f,.42f,6400,2.0f,14,22000,0},
       {"Dust",5100,.18f,.82f,.06f,.33f,4600,2.6f,12,12500,0},
       {"Metal",6800,.30f,.28f,.10f,.92f,11000,1.8f,14,28000,0},
       {"Loose",6200,.39f,.60f,.09f,.60f,8700,1.6f,14,26000,0},
       {"Soft",4700,.28f,.79f,.02f,.23f,4200,1.3f,15,24000,0},
       {"Crunch",6100,.16f,.63f,.15f,.71f,6500,3.3f,11,14000,0},
       {"Wash",5800,.34f,.91f,.03f,.33f,5700,1.7f,13,18000,0},
       {"Chop",8000,.075f,.44f,.21f,.73f,9200,2.4f,13,27000,0} }},
    {{ {"Body",95,.133f,.15f,.1f,.1f,10000,1,16,32000,57},
       {"Tape",88,.19f,.10f,.18f,.33f,4600,2.1f,14,22000,42},
       {"Floor",73,.29f,.08f,.16f,.40f,3800,2.3f,14,24000,35},
       {"Knock",108,.10f,.13f,.41f,.62f,6300,2.7f,13,26000,69},
       {"Hollow",84,.25f,.05f,.15f,.84f,4200,1.6f,15,28000,32},
       {"Crunch",96,.16f,.18f,.28f,.55f,5200,3.7f,11,14000,57},
       {"Dust",79,.22f,.14f,.16f,.40f,3200,2.4f,12,12500,38},
       {"Damped",119,.085f,.08f,.34f,.39f,5100,1.9f,14,25000,76},
       {"Round",68,.30f,.03f,.08f,.27f,2700,1.5f,15,24000,27},
       {"Tribal",103,.23f,.20f,.31f,.73f,6700,2.7f,13,21000,62} }},
    {{ {"Body",145,.133f,.15f,.1f,.1f,10000,1,16,32000,87},
       {"Tape",139,.16f,.10f,.18f,.34f,4700,2.1f,14,22000,65},
       {"Rack",122,.23f,.08f,.15f,.45f,4100,2.2f,14,24000,51},
       {"Knock",181,.085f,.12f,.40f,.64f,6600,2.7f,13,26000,110},
       {"Hollow",132,.21f,.05f,.14f,.85f,4500,1.6f,15,28000,48},
       {"Crunch",156,.13f,.18f,.28f,.56f,5400,3.7f,11,14000,93},
       {"Dust",128,.18f,.14f,.15f,.42f,3400,2.4f,12,12500,60},
       {"Damped",198,.065f,.07f,.35f,.41f,5400,1.9f,14,25000,123},
       {"Round",115,.25f,.03f,.08f,.28f,2900,1.5f,15,24000,45},
       {"Tribal",166,.19f,.21f,.31f,.76f,7000,2.7f,13,21000,100} }},
    {{ {"Classic",3100,.385f,.48f,.1f,.3f,12000,1,16,32000,0},
       {"Tape",2700,.36f,.61f,.06f,.49f,6500,2.0f,14,22000,0},
       {"Dust",2350,.31f,.79f,.04f,.38f,4600,2.5f,12,12500,0},
       {"Thin",4100,.29f,.38f,.12f,.77f,11200,1.5f,15,30000,0},
       {"Dark",2100,.50f,.67f,.03f,.52f,4800,1.8f,14,24000,0},
       {"Trash",3400,.40f,.49f,.20f,.92f,7900,3.4f,11,16000,0},
       {"Short",3600,.16f,.52f,.20f,.67f,8900,2.4f,13,26000,0},
       {"Soft",2500,.48f,.86f,.01f,.29f,5300,1.3f,15,25000,0},
       {"Crunch",3200,.28f,.66f,.14f,.74f,6100,3.7f,10,14000,0},
       {"Wash",2900,.64f,.91f,.02f,.37f,6800,1.6f,14,22000,0} }},
    {{ {"Bell",1560,.278f,.48f,.1f,.3f,12000,1,16,32000,0},
       {"Tape",1420,.25f,.39f,.06f,.70f,6300,2.0f,14,22000,0},
       {"Dust",1250,.23f,.58f,.04f,.50f,4500,2.5f,12,12500,0},
       {"Ping",1990,.18f,.19f,.21f,.94f,10500,1.8f,15,30000,0},
       {"Dark",1100,.36f,.48f,.03f,.62f,4100,1.6f,14,24000,0},
       {"Metal",1760,.35f,.31f,.13f,.90f,9500,2.2f,14,28000,0},
       {"Short",1860,.105f,.28f,.25f,.74f,8300,2.5f,13,26000,0},
       {"Soft",1320,.39f,.73f,.01f,.33f,5200,1.3f,15,25000,0},
       {"Crunch",1600,.22f,.54f,.15f,.79f,6200,3.6f,11,14000,0},
       {"Wash",1470,.47f,.87f,.02f,.45f,6700,1.7f,14,22000,0} }}
}};

class FactoryBank
{
public:
    using Variations = std::array<std::array<std::vector<float>, 3>, 3>;
    const std::vector<float>& sample(int drum, int preset, int layer, int rr) const noexcept
    { return samples[static_cast<size_t>(drum)][static_cast<size_t>(preset-1)][static_cast<size_t>(layer)][static_cast<size_t>(rr)]; }
    static const FactoryBank& get()
    {
        static const FactoryBank bank; // Prepared off the audio callback, shared by all instances.
        return bank;
    }
    size_t sampleBytes() const noexcept
    {
        size_t count = 0;
        for (const auto& drum : samples) for (const auto& preset : drum) for (const auto& layer : preset) for (const auto& rr : layer) count += rr.size()*sizeof(float);
        return count;
    }
private:
    std::array<std::array<Variations, factoryPresetCount-1>, 8> samples;
    FactoryBank()
    {
        for (int drum = 0; drum < 8; ++drum) for (int preset = 1; preset < factoryPresetCount; ++preset)
            for (int layer = 0; layer < 3; ++layer) for (int rr = 0; rr < 3; ++rr) generate(drum, preset, layer, rr);
    }
    void generate(int drum, int preset, int layer, int rr)
    {
        constexpr double pi = 3.14159265358979323846;
        const auto& p = factoryPresets[static_cast<size_t>(drum)][static_cast<size_t>(preset)];
        auto& out = samples[static_cast<size_t>(drum)][static_cast<size_t>(preset-1)][static_cast<size_t>(layer)][static_cast<size_t>(rr)];
        const auto duration = std::clamp(p.decay*7.0f+.025f, .16f, 3.4f);
        out.resize(static_cast<size_t>(duration*factorySampleRate));
        uint32_t rng = 8917u + static_cast<uint32_t>(drum*7919+preset*101+layer*313+rr*1187);
        const float variation = 1.0f+static_cast<float>(rr-1)*.009f;
        const float hardness = .67f+.165f*static_cast<float>(layer);
        const auto filter = 1.0-std::exp(-2*pi*std::min(14500.0f, p.cutoff*hardness)/factorySampleRate);
        const auto noiseFilter = 1.0-std::exp(-2*pi*800/factorySampleRate);
        double phase = 0, noiseLow = 0, low1 = 0, low2 = 0, dc = 0;
        float held = 0, peak = 0;
        double holdPosition = 1;
        const auto quantise = std::pow(2.0f, static_cast<float>(p.bits-1));
        for (size_t i = 0; i < out.size(); ++i)
        {
            const double t = static_cast<double>(i)/factorySampleRate;
            rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
            const auto noise = static_cast<double>(rng & 0xffffu)/32767.5-1;
            noiseLow += noiseFilter*(noise-noiseLow);
            const auto highNoise = noise-noiseLow;
            const auto bodyEnvelope = std::exp(-t/p.decay);
            double value = 0;
            if (drum == 0)
            {
                phase += 2*pi*(p.frequency+p.sweep*std::exp(-t*55))*variation/factorySampleRate;
                value = .83*std::sin(phase)*bodyEnvelope + p.harmonic*.28*std::sin(phase*1.97)*std::exp(-t/(p.decay*.48));
                value += p.noise*noise*std::exp(-t*160) + p.click*highNoise*hardness*std::exp(-t*310);
            }
            else if (drum == 1)
            {
                phase += 2*pi*p.frequency*variation/factorySampleRate;
                const auto shell = std::sin(phase)+p.harmonic*.65*std::sin(phase*1.53)+p.harmonic*.25*std::sin(phase*2.31);
                double wireEnvelope = std::exp(-t/p.decay);
                if (p.clap)
                    for (double delay : { .009, .017, .026 }) if (t >= delay) wireEnvelope += .75*std::exp(-(t-delay)/(.022+p.decay*.4));
                value = .34*shell*std::exp(-t/(p.decay*.7)) + p.noise*highNoise*wireEnvelope*hardness
                      + p.click*noise*std::exp(-t*240);
            }
            else if (drum == 4 || drum == 5)
            {
                phase += 2*pi*(p.frequency+p.sweep*std::exp(-t*35))*variation/factorySampleRate;
                value = .75*(std::sin(phase)+p.harmonic*.35*std::sin(phase*1.61))*bodyEnvelope
                      + p.harmonic*.13*std::sin(phase*2.47)*std::exp(-t/(p.decay*.5))
                      + p.noise*noise*std::exp(-t*75) + p.click*highNoise*hardness*std::exp(-t*260);
            }
            else if (drum == 2 || drum == 3)
            {
                double metal = 0;
                for (double ratio : { .43, .61, .83, 1.0, 1.31, 1.73 })
                    metal += (std::sin(2*pi*p.frequency*ratio*variation*t) >= 0 ? 1.0 : -1.0)/6;
                value = (p.noise*highNoise*.66+p.harmonic*metal*.70)*bodyEnvelope
                      + p.click*noise*std::exp(-t*360);
            }
            else
            {
                double metal = 0;
                int partial = 0;
                const double fundamental = drum == 6 ? p.frequency : p.frequency*1.9;
                for (double ratio : { .61, 1.0, 1.47, 2.11, 3.13, 4.03 })
                {
                    const auto frequency = std::min(14800.0, fundamental*ratio*variation);
                    metal += std::sin(2*pi*frequency*t)*std::exp(-t/(p.decay*(.75-.075*partial++)))/6;
                }
                value = p.noise*highNoise*.62*bodyEnvelope + p.harmonic*metal*.8
                      + p.click*noise*std::exp(-t*230);
                if (drum == 7) value += p.harmonic*.3*std::sin(2*pi*p.frequency*variation*t)*std::exp(-t/(p.decay*.6));
            }
            low1 += filter*(value-low1); low2 += filter*(low1-low2);
            value = std::tanh(low2*p.drive)/std::tanh(static_cast<double>(p.drive));
            holdPosition += p.holdRate/factorySampleRate;
            if (holdPosition >= 1) { held = static_cast<float>(std::round(value*quantise)/quantise); holdPosition -= std::floor(holdPosition); }
            dc += .002*(held-dc);
            const auto attack = std::min(1.0, t*2500);
            const auto tail = std::min(1.0, static_cast<double>(out.size()-i-1)/(factorySampleRate*.012));
            const auto sample = static_cast<float>((held-dc)*attack*tail);
            out[i] = sample; peak = std::max(peak, std::abs(sample));
        }
        // Match output peaks without flattening transient/envelope differences.
        const auto target = (.67f+.11f*static_cast<float>(layer));
        const auto level = target/std::max(.01f, peak);
        for (auto& sample : out) sample *= level;
    }
};
}
