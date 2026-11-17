#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace rabubanj
{
constexpr int factoryPresetCount = 20;
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
inline constexpr std::array<std::array<DrumPreset, 20>, 8> factoryPresets {{
    {{ {"Thump",48,.11f,.22f,.2f,.1f,9500,1,16,32000,115},
       {"Tape",52,.13f,.10f,.22f,.20f,4700,2.1f,14,22000,105},
       {"Sub",43,.24f,.04f,.09f,.08f,3100,1.5f,15,24000,78},
       {"Knock",64,.09f,.22f,.55f,.42f,6800,2.8f,13,24000,150},
       {"Hollow",57,.16f,.06f,.24f,.62f,4200,1.7f,15,28000,90},
       {"Crunch",55,.12f,.16f,.42f,.38f,5500,4.0f,11,14000,130},
       {"Dust",49,.14f,.14f,.18f,.27f,3300,2.4f,12,12500,100},
       {"Damped",68,.065f,.07f,.35f,.20f,4900,1.9f,14,26000,160},
       {"Round",46,.19f,.03f,.09f,.16f,2600,1.6f,15,24000,65},
       {"Slam",59,.155f,.18f,.44f,.50f,6100,3.3f,12,18000,145},
       {"Break",60,.105f,.11f,.28f,.31f,5800,2.2f,14,26000,120},
       {"Felt",51,.13f,.04f,.10f,.16f,3600,1.5f,15,28000,75},
       {"Knuckle",70,.075f,.13f,.47f,.41f,6200,2.6f,13,25000,145},
       {"DeepTap",44,.21f,.03f,.14f,.19f,3300,1.7f,15,28000,69},
       {"Warm",53,.16f,.06f,.18f,.28f,4200,1.8f,15,28000,95},
       {"Cabin",63,.11f,.10f,.26f,.55f,4800,2.0f,14,24000,117},
       {"Brick",59,.095f,.17f,.40f,.43f,5700,3.0f,13,22000,138},
       {"Velvet",47,.18f,.02f,.08f,.13f,2800,1.4f,16,32000,63},
       {"Boombox",56,.145f,.12f,.24f,.35f,4400,2.5f,12,18000,110},
       {"Punch",66,.083f,.08f,.39f,.29f,6600,2.4f,14,28000,155} }},
    {{ {"Back",185,.063f,.62f,.1f,.1f,10000,1,16,32000,0},
       {"Crack",224,.055f,.72f,.32f,.34f,8200,2.7f,14,26000,0},
       {"Wood",205,.075f,.32f,.48f,.68f,4300,2.1f,14,24000,0},
       {"Deep",146,.115f,.58f,.14f,.47f,6200,2.3f,14,24000,0},
       {"Clap",178,.075f,.87f,.12f,.25f,7100,2.0f,13,22000,0,true},
       {"Snap",244,.042f,.78f,.30f,.20f,10000,2.8f,14,30000,0},
       {"Grit",190,.085f,.73f,.25f,.46f,4800,4.2f,11,13000,0},
       {"Brush",172,.105f,.91f,.02f,.13f,5000,1.35f,15,26000,0},
       {"Wire",210,.14f,.89f,.18f,.30f,9000,1.9f,14,28000,0},
       {"Ring",232,.13f,.40f,.30f,.82f,6500,2.3f,13,21000,0},
       {"Chop",196,.075f,.69f,.21f,.38f,7100,1.8f,15,28000,0},
       {"Philly",178,.105f,.77f,.14f,.43f,6600,1.7f,15,28000,0},
       {"Oak",213,.087f,.44f,.29f,.61f,5300,1.8f,15,28000,0},
       {"Rim",241,.052f,.38f,.45f,.80f,7600,2.0f,14,26000,0},
       {"Fat",154,.13f,.68f,.16f,.49f,6100,2.1f,14,24000,0},
       {"Paper",228,.046f,.82f,.08f,.21f,8500,1.4f,16,32000,0},
       {"Vinyl2",187,.09f,.74f,.16f,.36f,5400,2.3f,13,19000,0},
       {"Room",170,.16f,.75f,.12f,.52f,7400,1.5f,15,28000,0},
       {"Dusty",181,.11f,.79f,.11f,.31f,4500,2.0f,13,18000,0},
       {"Back2",204,.082f,.66f,.26f,.46f,7800,1.9f,15,30000,0} }},
    {{ {"Tick",6500,.024f,.48f,.1f,.4f,12000,1,16,32000,0},
       {"Tight",7900,.016f,.40f,.13f,.66f,10800,1.7f,14,30000,0},
       {"Tape",5600,.029f,.65f,.03f,.38f,6600,2.0f,14,22000,0},
       {"Dust",5200,.023f,.81f,.05f,.35f,4700,2.5f,12,12500,0},
       {"Metal",6900,.038f,.28f,.08f,.90f,11000,1.9f,14,28000,0},
       {"Chick",7400,.018f,.61f,.27f,.46f,8500,2.3f,13,23000,0},
       {"Soft",4900,.045f,.76f,.01f,.24f,4100,1.3f,15,26000,0},
       {"Crunch",6300,.027f,.64f,.12f,.72f,6900,3.5f,10,14000,0},
       {"Vinyl",5700,.036f,.86f,.05f,.31f,5500,2.0f,12,16500,0},
       {"Cut",8200,.012f,.48f,.19f,.55f,9600,2.1f,13,28000,0},
       {"Silk",5800,.026f,.82f,.03f,.30f,7900,1.4f,16,32000,0},
       {"Brush",5100,.035f,.91f,.01f,.21f,6300,1.3f,15,28000,0},
       {"Bronze",6900,.031f,.62f,.08f,.67f,9600,1.5f,16,32000,0},
       {"Break",6400,.021f,.76f,.11f,.44f,8200,1.8f,15,28000,0},
       {"Pss",7600,.018f,.91f,.06f,.23f,10400,1.4f,16,32000,0},
       {"Mellow",4700,.04f,.83f,.01f,.28f,5200,1.3f,15,26000,0},
       {"Stick",7100,.023f,.55f,.22f,.64f,9300,1.8f,15,30000,0},
       {"Dry90",6200,.016f,.72f,.12f,.37f,7600,1.9f,14,24000,0},
       {"Sandy",5500,.03f,.88f,.05f,.32f,6500,1.7f,14,22000,0},
       {"Hush",4500,.027f,.89f,.01f,.18f,4700,1.2f,16,28000,0} }},
    {{ {"Sizzle",6500,.20f,.48f,.1f,.4f,12000,1,16,32000,0},
       {"Short",7700,.10f,.48f,.14f,.63f,10200,1.9f,14,30000,0},
       {"Tape",5600,.23f,.66f,.04f,.42f,6400,2.0f,14,22000,0},
       {"Dust",5100,.18f,.82f,.06f,.33f,4600,2.6f,12,12500,0},
       {"Metal",6800,.30f,.28f,.10f,.92f,11000,1.8f,14,28000,0},
       {"Loose",6200,.39f,.60f,.09f,.60f,8700,1.6f,14,26000,0},
       {"Soft",4700,.28f,.79f,.02f,.23f,4200,1.3f,15,24000,0},
       {"Crunch",6100,.16f,.63f,.15f,.71f,6500,3.3f,11,14000,0},
       {"Wash",5800,.34f,.91f,.03f,.33f,5700,1.7f,13,18000,0},
       {"Chop",8000,.075f,.44f,.21f,.73f,9200,2.4f,13,27000,0},
       {"Silk",5900,.22f,.85f,.03f,.28f,8500,1.3f,16,32000,0},
       {"Bronze",6900,.26f,.61f,.09f,.73f,10500,1.5f,16,32000,0},
       {"Sandy",5400,.18f,.88f,.05f,.33f,6700,1.6f,14,24000,0},
       {"Break",6400,.145f,.74f,.13f,.48f,8800,1.8f,15,28000,0},
       {"Wash2",6100,.35f,.92f,.02f,.31f,7900,1.3f,16,32000,0},
       {"DryAir",7500,.115f,.89f,.06f,.26f,9900,1.4f,16,30000,0},
       {"Dark2",4700,.24f,.79f,.02f,.36f,5400,1.5f,15,26000,0},
       {"Open90",6500,.19f,.76f,.11f,.55f,8700,1.8f,14,24000,0},
       {"Smooth",5700,.29f,.84f,.02f,.31f,7400,1.3f,16,32000,0},
       {"Shhh",5100,.21f,.94f,.01f,.17f,6200,1.2f,16,28000,0} }},
    {{ {"Body",95,.133f,.15f,.1f,.1f,10000,1,16,32000,57},
       {"Tape",88,.19f,.10f,.18f,.33f,4600,2.1f,14,22000,42},
       {"Floor",73,.29f,.08f,.16f,.40f,3800,2.3f,14,24000,35},
       {"Knock",108,.10f,.13f,.41f,.62f,6300,2.7f,13,26000,69},
       {"Hollow",84,.25f,.05f,.15f,.84f,4200,1.6f,15,28000,32},
       {"Crunch",96,.16f,.18f,.28f,.55f,5200,3.7f,11,14000,57},
       {"Dust",79,.22f,.14f,.16f,.40f,3200,2.4f,12,12500,38},
       {"Damped",119,.085f,.08f,.34f,.39f,5100,1.9f,14,25000,76},
       {"Round",68,.30f,.03f,.08f,.27f,2700,1.5f,15,24000,27},
       {"Tribal",103,.23f,.20f,.31f,.73f,6700,2.7f,13,21000,62},
       {"Maple",91,.17f,.08f,.22f,.43f,5300,1.7f,15,28000,47},
       {"Skin",101,.14f,.11f,.27f,.58f,6100,1.6f,15,30000,58},
       {"Dry90",112,.09f,.09f,.31f,.35f,5200,2.0f,14,24000,70},
       {"Satin",82,.23f,.04f,.13f,.38f,3900,1.4f,16,32000,36},
       {"Thud",76,.135f,.07f,.19f,.25f,3400,2.1f,14,24000,40},
       {"Wood2",107,.12f,.12f,.35f,.68f,5800,1.9f,15,28000,61},
       {"Root",65,.26f,.02f,.09f,.24f,2900,1.5f,16,32000,28},
       {"Lounge",87,.20f,.06f,.14f,.46f,4400,1.4f,16,30000,41},
       {"Shell",97,.18f,.10f,.24f,.79f,5900,1.7f,15,28000,53},
       {"Warm",80,.22f,.05f,.12f,.34f,3500,1.6f,15,26000,35} }},
    {{ {"Body",145,.133f,.15f,.1f,.1f,10000,1,16,32000,87},
       {"Tape",139,.16f,.10f,.18f,.34f,4700,2.1f,14,22000,65},
       {"Rack",122,.23f,.08f,.15f,.45f,4100,2.2f,14,24000,51},
       {"Knock",181,.085f,.12f,.40f,.64f,6600,2.7f,13,26000,110},
       {"Hollow",132,.21f,.05f,.14f,.85f,4500,1.6f,15,28000,48},
       {"Crunch",156,.13f,.18f,.28f,.56f,5400,3.7f,11,14000,93},
       {"Dust",128,.18f,.14f,.15f,.42f,3400,2.4f,12,12500,60},
       {"Damped",198,.065f,.07f,.35f,.41f,5400,1.9f,14,25000,123},
       {"Round",115,.25f,.03f,.08f,.28f,2900,1.5f,15,24000,45},
       {"Tribal",166,.19f,.21f,.31f,.76f,7000,2.7f,13,21000,100},
       {"Maple",142,.135f,.08f,.22f,.44f,5500,1.7f,15,28000,70},
       {"Skin",161,.11f,.11f,.27f,.59f,6300,1.6f,15,30000,92},
       {"Dry90",182,.07f,.09f,.31f,.36f,5500,2.0f,14,24000,111},
       {"Satin",129,.19f,.04f,.13f,.39f,4200,1.4f,16,32000,55},
       {"Thud",119,.105f,.07f,.19f,.26f,3700,2.1f,14,24000,61},
       {"Wood2",172,.095f,.12f,.35f,.69f,6100,1.9f,15,28000,100},
       {"Root",106,.21f,.02f,.09f,.25f,3200,1.5f,16,32000,45},
       {"Lounge",135,.16f,.06f,.14f,.47f,4700,1.4f,16,30000,63},
       {"Shell",153,.14f,.10f,.24f,.80f,6200,1.7f,15,28000,84},
       {"Warm",124,.18f,.05f,.12f,.35f,3800,1.6f,15,26000,51} }},
    {{ {"Classic",3100,.385f,.48f,.1f,.3f,12000,1,16,32000,0},
       {"Tape",2700,.36f,.61f,.06f,.49f,6500,2.0f,14,22000,0},
       {"Dust",2350,.31f,.79f,.04f,.38f,4600,2.5f,12,12500,0},
       {"Thin",4100,.29f,.38f,.12f,.77f,11200,1.5f,15,30000,0},
       {"Dark",2100,.50f,.67f,.03f,.52f,4800,1.8f,14,24000,0},
       {"Trash",3400,.40f,.49f,.20f,.92f,7900,3.4f,11,16000,0},
       {"Short",3600,.16f,.52f,.20f,.67f,8900,2.4f,13,26000,0},
       {"Soft",2500,.48f,.86f,.01f,.29f,5300,1.3f,15,25000,0},
       {"Crunch",3200,.28f,.66f,.14f,.74f,6100,3.7f,10,14000,0},
       {"Wash",2900,.64f,.91f,.02f,.37f,6800,1.6f,14,22000,0},
       {"Bronze",3000,.43f,.81f,.05f,.55f,10000,1.4f,16,32000,0},
       {"Break",3300,.29f,.78f,.11f,.48f,8700,1.7f,15,28000,0},
       {"Velvet",2500,.45f,.87f,.02f,.29f,7100,1.2f,16,32000,0},
       {"Paper",3900,.25f,.91f,.09f,.36f,10500,1.3f,16,32000,0},
       {"Room",2750,.51f,.78f,.04f,.61f,9200,1.4f,16,32000,0},
       {"Dust90",2600,.34f,.84f,.05f,.39f,5900,1.8f,14,22000,0},
       {"Brass",3500,.38f,.66f,.10f,.78f,11000,1.6f,16,32000,0},
       {"Air",3200,.48f,.95f,.02f,.24f,11200,1.2f,16,32000,0},
       {"Ribbon",2300,.42f,.89f,.01f,.31f,6200,1.3f,16,28000,0},
       {"Soft90",2800,.46f,.88f,.03f,.42f,7700,1.4f,15,26000,0} }},
    {{ {"Bell",1560,.278f,.48f,.1f,.3f,12000,1,16,32000,0},
       {"Tape",1420,.25f,.39f,.06f,.70f,6300,2.0f,14,22000,0},
       {"Dust",1250,.23f,.58f,.04f,.50f,4500,2.5f,12,12500,0},
       {"Ping",1990,.18f,.19f,.21f,.94f,10500,1.8f,15,30000,0},
       {"Dark",1100,.36f,.48f,.03f,.62f,4100,1.6f,14,24000,0},
       {"Metal",1760,.35f,.31f,.13f,.90f,9500,2.2f,14,28000,0},
       {"Short",1860,.105f,.28f,.25f,.74f,8300,2.5f,13,26000,0},
       {"Soft",1320,.39f,.73f,.01f,.33f,5200,1.3f,15,25000,0},
       {"Crunch",1600,.22f,.54f,.15f,.79f,6200,3.6f,11,14000,0},
       {"Wash",1470,.47f,.87f,.02f,.45f,6700,1.7f,14,22000,0},
       {"Bronze",1540,.31f,.48f,.07f,.72f,9400,1.4f,16,32000,0},
       {"Jazz",1390,.34f,.42f,.08f,.63f,8100,1.3f,16,32000,0},
       {"Soft90",1280,.29f,.68f,.03f,.45f,6500,1.5f,15,26000,0},
       {"Stick",1850,.16f,.31f,.25f,.83f,10100,1.7f,16,32000,0},
       {"Velvet",1190,.37f,.73f,.01f,.32f,5900,1.2f,16,32000,0},
       {"Edge",1660,.28f,.61f,.11f,.70f,9600,1.6f,15,30000,0},
       {"Dry90",1770,.13f,.40f,.20f,.60f,7800,1.8f,14,24000,0},
       {"Dark2",1080,.35f,.66f,.02f,.40f,5100,1.3f,16,28000,0},
       {"Air",1490,.40f,.85f,.02f,.33f,10200,1.2f,16,32000,0},
       {"Room",1430,.41f,.61f,.05f,.58f,8500,1.4f,16,30000,0} }}
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
        struct Mode { double a1, a2, y1, y2; };
        std::array<Mode, 18> modes {};
        const int modeCount = drum == 6 ? 18 : drum == 7 ? 14 : (drum == 2 || drum == 3 ? 12 : 0);
        for (int mode = 0; mode < modeCount; ++mode)
        {
            rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
            const auto random = static_cast<double>(rng & 0xffffu)/65535.0;
            const auto fraction = static_cast<double>(mode)/std::max(1, modeCount-1);
            const auto base = (drum == 2 || drum == 3) ? p.frequency*.42 : p.frequency*(drum == 6 ? .33 : .7);
            auto frequency = base*.6*std::pow(drum == 6 ? 22.0 : 18.0, fraction)*(1+(random-.5)*.027)*variation;
            if (drum == 7 && mode == 0) frequency = p.frequency*variation;
            frequency = std::clamp(frequency, 250.0, 14200.0);
            const auto decay = p.decay*(.65+random*.85)/(1+mode*.045);
            const auto radius = std::exp(-1/(factorySampleRate*decay));
            const auto angle = 2*pi*frequency/factorySampleRate, initialPhase = random*2*pi;
            const auto amplitude = .36/std::sqrt(static_cast<double>(modeCount))/(1+mode*.08);
            modes[static_cast<size_t>(mode)] = { 2*radius*std::cos(angle), -radius*radius,
                amplitude*std::sin(initialPhase), amplitude*std::sin(initialPhase-angle)/radius };
        }
        double phase = 0, noiseLow = 0, noiseMid = 0, low1 = 0, low2 = 0, dc = 0;
        const auto midFilter = 1.0-std::exp(-2*pi*4200/factorySampleRate);
        const auto effectiveDrive = (drum == 1 || drum == 2 || drum == 3 || drum >= 6) ? 1+(p.drive-1)*.6 : p.drive;
        const auto holdRate = (drum == 1 || drum == 3 || drum >= 6) ? std::max(16000.0f, p.holdRate) : p.holdRate;
        const auto bits = (drum == 1 || drum == 3 || drum >= 6) ? std::max(12, p.bits) : p.bits;
        float held = 0, peak = 0;
        double holdPosition = 1;
        const auto quantise = std::pow(2.0f, static_cast<float>(bits-1));
        for (size_t i = 0; i < out.size(); ++i)
        {
            const double t = static_cast<double>(i)/factorySampleRate;
            rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
            const auto noise = static_cast<double>(rng & 0xffffu)/32767.5-1;
            noiseLow += noiseFilter*(noise-noiseLow);
            noiseMid += midFilter*(noise-noiseMid);
            const auto highNoise = noise-noiseLow;
            const auto bandNoise = noiseMid-noiseLow;
            double modal = 0;
            for (int mode = 0; mode < modeCount; ++mode)
            {
                auto& resonator = modes[static_cast<size_t>(mode)];
                const auto value = resonator.a1*resonator.y1+resonator.a2*resonator.y2;
                resonator.y2 = resonator.y1; resonator.y1 = value; modal += value;
            }
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
                phase += 2*pi*p.frequency*(1+.055*std::exp(-t*80))*variation/factorySampleRate;
                const auto shell = std::sin(phase)+p.harmonic*.48*std::sin(phase*1.83)+p.harmonic*.18*std::sin(phase*2.78);
                double wireEnvelope = .78*std::exp(-t/(p.decay*.75))+.22*std::exp(-t/(p.decay*1.35));
                if (p.clap)
                    for (double delay : { .009, .017, .026 }) if (t >= delay) wireEnvelope += .75*std::exp(-(t-delay)/(.022+p.decay*.4));
                value = .40*shell*std::exp(-t/(p.decay*.56)) + p.noise*(highNoise*.64+bandNoise*.36)*wireEnvelope*hardness
                      + p.click*noise*std::exp(-t*290);
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
                const auto envelope = drum == 3 ? .85*bodyEnvelope+.15*std::exp(-t/(p.decay*1.6)) : bodyEnvelope;
                const auto onset = 1-std::exp(-t/(drum == 3 ? .0018 : .00045));
                value = ((highNoise*.72+bandNoise*.28)*p.noise*.72*envelope+p.harmonic*modal*.38)*onset
                      + p.click*highNoise*.4*std::exp(-t*360);
            }
            else
            {
                const auto envelope = .83*bodyEnvelope+.17*std::exp(-t/(p.decay*1.6));
                const auto onset = 1-std::exp(-t/(drum == 6 ? .004 : .001));
                value = (p.noise*(highNoise*.72+bandNoise*.28)*.72*envelope+p.harmonic*modal*.65)*onset
                      + p.click*highNoise*.35*std::exp(-t*260);
                if (drum == 7) value += p.harmonic*.11*std::sin(2*pi*p.frequency*variation*t)*std::exp(-t/(p.decay*.55));
            }
            low1 += filter*(value-low1); low2 += filter*(low1-low2);
            value = std::tanh(low2*effectiveDrive)/std::tanh(static_cast<double>(effectiveDrive));
            holdPosition += holdRate/factorySampleRate;
            if (holdPosition >= 1) { held = static_cast<float>(std::round(value*quantise)/quantise); holdPosition -= std::floor(holdPosition); }
            dc += .002*(held-dc);
            const auto attack = std::min(1.0, t*2500);
            const auto tail = std::min(1.0, static_cast<double>(out.size()-i-1)/(factorySampleRate*.03));
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
