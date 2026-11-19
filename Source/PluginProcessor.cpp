#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout RaBubanjProcessor::makeParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    auto addFloat = [&](const juce::String& id, const juce::String& name, juce::NormalisableRange<float> range, float value)
    { layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { id, 1 }, name, range, value)); };
    auto addBool = [&](const juce::String& id, const juce::String& name, bool value = false)
    { layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { id, 1 }, name, value)); };
    for (int d = 0; d < rabubanj::drumCount; ++d)
    {
        const auto id = juce::String(d), name = juce::String(rabubanj::names[static_cast<size_t>(d)]);
        addFloat("gain" + id, name + " Level", { -48.0f, 6.0f, .1f }, 0);
        const rabubanj::Mix defaults;
        addFloat("pan" + id, name + " Pan", { -1.0f, 1.0f, .01f }, defaults.pan[static_cast<size_t>(d)]);
        addBool("mute" + id, name + " Mute");
        addFloat("send" + id, name + " Reverb Send", { 0.0f, 1.0f, .001f }, 0.0f);
        addBool("solo" + id, name + " Solo");
        juce::StringArray presetNames;
        for (int preset = 0; preset < 10; ++preset)
            presetNames.add(juce::String(preset+1).paddedLeft('0', 2)+" "+rabubanj::factoryPresets[static_cast<size_t>(d)][static_cast<size_t>(preset)].name);
        layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "preset"+id, 1 }, name+" Factory Preset", presetNames, 0));
        addBool("presetBank"+id, name+" Factory Bank 11-20");
    }
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "midiMap", 1 }, "MIDI Map",
        juce::StringArray { "Chromatic C1-G1", "GM Legacy" }, 0));
    addFloat("master", "Master Level", { -60.0f, 6.0f, .1f }, -8);
    // Retain the 0.1 room ID and its linear amplitude domain for DAW compatibility.
    addFloat("room", "Reverb Return", { 0.0f, 1.0f, .0001f }, .12f);
    addBool("reverbMute", "Reverb Mute"); addBool("solo8", "Reverb Solo"); addBool("masterMute", "Master Mute");
    addFloat("roomSize", "Reverb Size", { 0.0f, 1.0f, .001f }, .45f);
    addFloat("damping", "Reverb Damping", { 0.0f, 1.0f, .001f }, .55f);
    addFloat("width", "Reverb Width", { 0.0f, 1.0f, .001f }, .9f);
    for (int bus = 0; bus < rabubanj::eqBusCount; ++bus)
    {
        const auto name = bus < 8 ? juce::String(rabubanj::names[static_cast<size_t>(bus)]) : bus == 8 ? "Reverb" : "Master";
        addBool("eqBypass" + juce::String(bus), name + " EQ Bypass");
        for (int band = 0; band < rabubanj::eqBandCount; ++band)
        {
            auto id = [&](const char* field) { return juce::String(rabubanj::eqParameterId(bus, band, field)); };
            const auto prefix = name + " EQ " + juce::String(band + 1) + " ";
            addFloat(id("freq"), prefix + "Frequency", { 20.0f, 20000.0f, .01f, .25f }, rabubanj::eqFrequencies[static_cast<size_t>(band)]);
            addFloat(id("gain"), prefix + "Gain", { -24.0f, 24.0f, .01f }, 0);
            addFloat(id("q"), prefix + "Q", { .15f, 18.0f, .01f, .35f }, .7071f);
            layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { id("type"), 1 }, prefix + "Type",
                juce::StringArray { "Bell", "Low shelf", "High shelf", "Low cut", "High cut", "Notch" }, 0));
            addBool(id("on"), prefix + "Enabled", true);
        }
    }
    for (int drum = 0; drum < 8; ++drum)
    {
        const auto id = juce::String(drum), name = juce::String(rabubanj::names[static_cast<size_t>(drum)]);
        addFloat("pitch"+id, name+" Pitch", { -24.0f, 24.0f, .01f }, 0);
        addFloat("drive"+id, name+" Drive", { 0.0f, 1.0f, .001f }, 0);
        addFloat("comp"+id, name+" Compression", { 0.0f, 1.0f, .001f }, 0);
    }
    return layout;
}

RaBubanjProcessor::RaBubanjProcessor()
    : AudioProcessor(BusesProperties().withOutput("Stereo", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "RaBubanjState", makeParameters())
{
    for (int d = 0; d < rabubanj::drumCount; ++d)
    {
        const auto i = static_cast<size_t>(d); const auto id = juce::String(d);
        gainParams[i] = parameters.getRawParameterValue("gain" + id);
        panParams[i] = parameters.getRawParameterValue("pan" + id);
        muteParams[i] = parameters.getRawParameterValue("mute" + id);
        sendParams[i] = parameters.getRawParameterValue("send" + id);
        presetParams[i] = parameters.getRawParameterValue("preset" + id);
        presetBankParams[i] = parameters.getRawParameterValue("presetBank" + id);
        pitchParams[i] = parameters.getRawParameterValue("pitch"+id);
        driveParams[i] = parameters.getRawParameterValue("drive"+id);
        compParams[i] = parameters.getRawParameterValue("comp"+id);
    }
    for (int i = 0; i <= rabubanj::reverbBus; ++i) soloParams[static_cast<size_t>(i)] = parameters.getRawParameterValue("solo" + juce::String(i));
    for (int bus = 0; bus < rabubanj::eqBusCount; ++bus)
    {
        auto& eq = eqParams[static_cast<size_t>(bus)];
        eq.bypass = parameters.getRawParameterValue("eqBypass" + juce::String(bus));
        for (int band = 0; band < rabubanj::eqBandCount; ++band)
        {
            auto get = [&](const char* field) { return parameters.getRawParameterValue(juce::String(rabubanj::eqParameterId(bus, band, field))); };
            eq.bands[static_cast<size_t>(band)] = { get("freq"), get("gain"), get("q"), get("type"), get("on") };
        }
    }
    for (auto& tap : spectrumTaps) tap = std::make_unique<rabubanj::SpectrumTap>();
    midiMapParam = parameters.getRawParameterValue("midiMap");
    sampleLoader.onChange = [this] { triggerAsyncUpdate(); };
    masterParam = parameters.getRawParameterValue("master"); roomParam = parameters.getRawParameterValue("room");
    reverbMuteParam = parameters.getRawParameterValue("reverbMute"); masterMuteParam = parameters.getRawParameterValue("masterMute");
    sizeParam = parameters.getRawParameterValue("roomSize"); dampingParam = parameters.getRawParameterValue("damping"); widthParam = parameters.getRawParameterValue("width");
}

RaBubanjProcessor::~RaBubanjProcessor()
{
    sampleLoader.shutdown(); cancelPendingUpdate(); engine.reset();
}
int RaBubanjProcessor::legacySource(int drum) const noexcept
{
    const auto i = static_cast<size_t>(drum);
    return juce::jlimit(0, 9, static_cast<int>(presetParams[i]->load()))+(presetBankParams[i]->load() >= .5f ? 10 : 0);
}

rabubanj::EqSettings RaBubanjProcessor::getEqSettings(int bus) const noexcept
{
    rabubanj::EqSettings settings;
    const auto& eq = eqParams[static_cast<size_t>(juce::jlimit(0, rabubanj::masterBus, bus))];
    settings.bypass = eq.bypass->load() >= .5f;
    for (size_t i = 0; i < settings.bands.size(); ++i)
    {
        const auto& p = eq.bands[i];
        settings.bands[i] = { p.frequency->load(), p.gain->load(), p.q->load(), static_cast<int>(p.type->load()), p.enabled->load() >= .5f };
    }
    return settings;
}
void RaBubanjProcessor::prepareToPlay(double rate, int)
{
    displaySampleRate.store(rate);
    engine.prepare(rate); reverb.setSampleRate(rate); reverb.reset();
    for (auto& eq : equalisers) eq.prepare(rate);
    masterGain.reset(rate, .02); masterGain.setCurrentAndTargetValue(masterMuteParam->load() >= .5f ? 0 : juce::Decibels::decibelsToGain(masterParam->load()));
    returnGain.reset(rate, .02); returnGain.setCurrentAndTargetValue(reverbMuteParam->load() >= .5f ? 0 : roomParam->load());
    bool anySolo = false;
    for (auto* solo : soloParams) anySolo = anySolo || solo->load() >= .5f;
    for (size_t i = 0; i < gains.size(); ++i)
    {
        const bool audible = muteParams[i]->load() < .5f && (!anySolo || soloParams[i]->load() >= .5f || soloParams[8]->load() >= .5f);
        gains[i].reset(rate, .015); gains[i].setCurrentAndTargetValue(audible ? juce::Decibels::decibelsToGain(gainParams[i]->load()) : 0);
        dryLevels[i].reset(rate, .015); dryLevels[i].setCurrentAndTargetValue(!anySolo || soloParams[i]->load() >= .5f ? 1.0f : 0.0f);
        pans[i].reset(rate, .015); pans[i].setCurrentAndTargetValue(panParams[i]->load());
        sends[i].reset(rate, .015); sends[i].setCurrentAndTargetValue(sendParams[i]->load());
        channelColour[i].set(driveParams[i]->load(), compParams[i]->load()); channelColour[i].prepare(rate);
    }
    for (auto& peak : peaks) peak.store(0);
}
void RaBubanjProcessor::resetAudio() noexcept
{
    engine.reset(); reverb.reset(); for (auto& eq : equalisers) eq.reset();
    for (auto& colour : channelColour) colour.reset();
}
void RaBubanjProcessor::releaseResources() { resetAudio(); }
bool RaBubanjProcessor::isBusesLayoutSupported(const BusesLayout& layout) const
{
    return layout.getMainInputChannelSet().isDisabled() && layout.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
void RaBubanjProcessor::audition(int drum) noexcept
{
    if (drum < 0 || drum >= rabubanj::drumCount) return;
    int start1, size1, start2, size2;
    padFifo.prepareToWrite(1, start1, size1, start2, size2);
    if (size1 > 0) padQueue[static_cast<size_t>(start1)] = drum;
    padFifo.finishedWrite(size1);
}
void RaBubanjProcessor::hit(int note, float velocity) noexcept { hitDrum(engine.drumForIncomingNote(note), velocity); }
void RaBubanjProcessor::hitDrum(int drum, float velocity) noexcept
{
    if (drum < 0 || drum >= 8 || velocity <= 0.0f) return;
    engine.triggerDrum(drum, velocity); hits[static_cast<size_t>(drum)].fetch_add(1, std::memory_order_relaxed);
}
void RaBubanjProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    if (buffer.getNumChannels() < 2 || buffer.getNumSamples() == 0) { midi.clear(); return; }
    const int samples = buffer.getNumSamples();
    auto* snapshot = sampleLoader.beginAudioRead();
    struct ReadScope { SampleLoader& loader; ~ReadScope() { loader.endAudioRead(); } } readScope { sampleLoader };
    engine.setLegacyMidi(midiMapParam->load() >= .5f);
    hostRecording.store(false);
    if (auto* hostPlayHead = getPlayHead()) if (auto position = hostPlayHead->getPosition()) hostRecording.store(position->getIsRecording());
    for (int drum = 0; drum < rabubanj::drumCount; ++drum)
    {
        engine.setPreset(drum, legacySource(drum));
        engine.setPitch(drum, pitchParams[static_cast<size_t>(drum)]->load());
        channelColour[static_cast<size_t>(drum)].set(driveParams[static_cast<size_t>(drum)]->load(), compParams[static_cast<size_t>(drum)]->load());
        engine.setCustomSample(drum, sampleLoader.audioSource(*snapshot, drum), sampleLoader.usesCustom(drum));
    }
    bool anySolo = false;
    for (auto* solo : soloParams) anySolo = anySolo || solo->load() >= .5f;
    const bool returnSolo = soloParams[8]->load() >= .5f;
    std::array<bool, 8> dryAudible {};
    for (size_t i = 0; i < gains.size(); ++i)
    {
        const bool solo = soloParams[i]->load() >= .5f;
        const bool audible = muteParams[i]->load() < .5f && (!anySolo || solo || returnSolo);
        dryAudible[i] = !anySolo || solo;
        dryLevels[i].setTargetValue(dryAudible[i] ? 1.0f : 0.0f);
        gains[i].setTargetValue(audible ? juce::Decibels::decibelsToGain(gainParams[i]->load()) : 0);
        pans[i].setTargetValue(panParams[i]->load()); sends[i].setTargetValue(sendParams[i]->load());
    }
    masterGain.setTargetValue(masterMuteParam->load() >= .5f ? 0 : juce::Decibels::decibelsToGain(masterParam->load()));
    returnGain.setTargetValue(reverbMuteParam->load() >= .5f ? 0 : roomParam->load());
    for (int bus = 0; bus < rabubanj::eqBusCount; ++bus) equalisers[static_cast<size_t>(bus)].setSettings(getEqSettings(bus));
    juce::Reverb::Parameters room;
    room.roomSize = sizeParam->load(); room.damping = dampingParam->load(); room.width = widthParam->load();
    room.wetLevel = .3f; room.dryLevel = 0;
    reverb.setParameters(room);
    int start1, size1, start2, size2;
    padFifo.prepareToRead(padFifo.getNumReady(), start1, size1, start2, size2);
    for (int i = 0; i < size1; ++i) hitDrum(padQueue[static_cast<size_t>(start1 + i)], .85f);
    for (int i = 0; i < size2; ++i) hitDrum(padQueue[static_cast<size_t>(start2 + i)], .85f);
    padFifo.finishedRead(size1 + size2);
    const auto analysisBus = analyserActive.load() ? getSelectedChannel() : -1;
    const bool showAnalysis = analyserActive.load();
    auto* left = buffer.getWritePointer(0); auto* right = buffer.getWritePointer(1);
    std::array<float, rabubanj::eqBusCount> blockPeaks {};
    auto renderSegment = [&](int offset, int length)
    {
        while (length > 0)
        {
            const int count = std::min(chunkSize, length);
            std::array<float*, 8> outputs, rightOutputs;
            for (size_t d = 0; d < stems.size(); ++d)
            {
                std::fill_n(stems[d].data(), count, 0.0f); std::fill_n(rightStems[d].data(), count, 0.0f);
                outputs[d] = stems[d].data(); rightOutputs[d] = rightStems[d].data();
            }
            engine.renderStereoStems(outputs, rightOutputs, count);
            std::fill_n(wetLeft.data(), count, 0.0f); std::fill_n(wetRight.data(), count, 0.0f);
            for (size_t d = 0; d < stems.size(); ++d)
                for (int s = 0; s < count; ++s)
                {
                    auto sourceLeft = stems[d][static_cast<size_t>(s)], sourceRight = rightStems[d][static_cast<size_t>(s)];
                    channelColour[d].process(sourceLeft, sourceRight);
                    equalisers[d].processStereo(sourceLeft, sourceRight);
                    stems[d][static_cast<size_t>(s)] = sourceLeft; rightStems[d][static_cast<size_t>(s)] = sourceRight;
                    const auto level = gains[d].getNextValue();
                    const auto angle = (pans[d].getNextValue() + 1.0f) * juce::MathConstants<float>::pi * .25f;
                    const auto l = sourceLeft*level*std::cos(angle), r = sourceRight*level*std::sin(angle);
                    const auto send = sends[d].getNextValue();
                    wetLeft[static_cast<size_t>(s)] += l * send; wetRight[static_cast<size_t>(s)] += r * send;
                    // Preserve the 0.1 dry calibration (JUCE Reverb dryLevel=1 had a factor of 2).
                    const auto dry = dryLevels[d].getNextValue() * 2;
                    left[offset+s] += l * dry; right[offset+s] += r * dry;
                    blockPeaks[d] = std::max(blockPeaks[d], std::max(std::abs(l), std::abs(r)) * 2);
                }
            if (analysisBus >= 0 && analysisBus < 8) spectrumTaps[static_cast<size_t>(analysisBus)]->push(stems[static_cast<size_t>(analysisBus)].data(), rightStems[static_cast<size_t>(analysisBus)].data(), count);
            reverb.processStereo(wetLeft.data(), wetRight.data(), count);
            for (int s = 0; s < count; ++s)
            {
                float l = wetLeft[static_cast<size_t>(s)], r = wetRight[static_cast<size_t>(s)];
                equalisers[8].processStereo(l, r);
                const auto returnLevel = returnGain.getNextValue(); l *= returnLevel; r *= returnLevel;
                wetLeft[static_cast<size_t>(s)] = l; wetRight[static_cast<size_t>(s)] = r;
                blockPeaks[8] = std::max(blockPeaks[8], std::max(std::abs(l), std::abs(r)));
                l += left[offset+s]; r += right[offset+s];
                equalisers[9].processStereo(l, r);
                const auto gain = masterGain.getNextValue();
                left[offset+s] = std::tanh(l * gain); right[offset+s] = std::tanh(r * gain);
                blockPeaks[9] = std::max(blockPeaks[9], std::max(std::abs(left[offset+s]), std::abs(right[offset+s])));
            }
            if (analysisBus == 8) spectrumTaps[8]->push(wetLeft.data(), wetRight.data(), count);
            if (showAnalysis) spectrumTaps[9]->push(left+offset, right+offset, count);
            offset += count; length -= count;
        }
    };
    int cursor = 0;
    for (const auto metadata : midi)
    {
        const auto position = juce::jlimit(cursor, samples, metadata.samplePosition);
        renderSegment(cursor, position - cursor); cursor = position;
        if (metadata.numBytes > 3) continue;
        const auto message = metadata.getMessage();
        if (message.isNoteOn()) hit(message.getNoteNumber(), message.getFloatVelocity());
        else if (message.isAllNotesOff() || message.isAllSoundOff()) resetAudio();
    }
    renderSegment(cursor, samples - cursor); midi.clear();
    const auto decay = static_cast<float>(std::exp(-static_cast<double>(samples) / (displaySampleRate.load() * .3)));
    for (size_t i = 0; i < peaks.size(); ++i) peaks[i].store(std::max(blockPeaks[i], peaks[i].load() * decay));
}
void RaBubanjProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    auto state = parameters.copyState();
    state.setProperty("schemaVersion", 5, nullptr);
    state.setProperty("localKeys", localKeyboard.load(), nullptr);
    if (auto oldSamples = state.getChildWithName("SAMPLES"); oldSamples.isValid()) state.removeChild(oldSamples, nullptr);
    state.addChild(sampleLoader.save(), -1, nullptr); state.setProperty("selectedChannel", selectedChannel.load(), nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary(*xml, destination);
}
void RaBubanjProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(parameters.state.getType()))
        {
            auto state = juce::ValueTree::fromXml(*xml);
            if (static_cast<int>(state.getProperty("schemaVersion", 1)) < 4)
                for (int drum = 0; drum < 8; ++drum)
                {
                    auto send = state.getChildWithProperty("id", "send"+juce::String(drum));
                    if (send.isValid() && std::abs(static_cast<float>(send.getProperty("value"))-.25f) < .0001f) send.setProperty("value", 0.0f, nullptr);
                }
            // Load older projects with unity sends, reproducing their shared room feed.
            if (static_cast<int>(state.getProperty("schemaVersion", 1)) < 2)
                for (int d = 0; d < 8; ++d)
                {
                    if (state.getChildWithProperty("id", "send" + juce::String(d)).isValid()) continue;
                    juce::ValueTree send("PARAM"); send.setProperty("id", "send" + juce::String(d), nullptr);
                    send.setProperty("value", 1.0f, nullptr); state.addChild(send, -1, nullptr);
                }
            for (auto* base : getParameters())
                if (auto* parameter = dynamic_cast<juce::RangedAudioParameter*>(base))
                    if (!state.getChildWithProperty("id", parameter->paramID).isValid())
                    {
                        juce::ValueTree value("PARAM"); value.setProperty("id", parameter->paramID, nullptr);
                        value.setProperty("value", parameter->convertFrom0to1(parameter->getDefaultValue()), nullptr);
                        state.addChild(value, -1, nullptr);
                    }
            selectChannel(static_cast<int>(state.getProperty("selectedChannel", 0)));
            localKeyboard.store(static_cast<bool>(state.getProperty("localKeys", false)));
            parameters.replaceState(state);
            sampleLoader.restore(state.getChildWithName("SAMPLES"));
        }
}
juce::AudioProcessorEditor* RaBubanjProcessor::createEditor() { return new RaBubanjEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RaBubanjProcessor(); }
