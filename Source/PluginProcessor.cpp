#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout RaBubanjProcessor::makeParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (int d = 0; d < rabubanj::drumCount; ++d)
    {
        const auto id = juce::String(d);
        const auto name = juce::String(rabubanj::names[static_cast<size_t>(d)]);
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "gain" + id, 1 }, name + " Level",
            juce::NormalisableRange<float>(-48.0f, 6.0f, .1f), 0.0f));
        const rabubanj::Mix defaults;
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "pan" + id, 1 }, name + " Pan",
            juce::NormalisableRange<float>(-1.0f, 1.0f, .01f), defaults.pan[static_cast<size_t>(d)]));
        layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "mute" + id, 1 }, name + " Mute", false));
    }
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "master", 1 }, "Master Level",
        juce::NormalisableRange<float>(-60.0f, 6.0f, .1f), -8.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "room", 1 }, "Room Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, .01f), .12f));
    return layout;
}

RaBubanjProcessor::RaBubanjProcessor()
    : AudioProcessor(BusesProperties().withOutput("Stereo", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "RaBubanjState", makeParameters())
{
    for (int d = 0; d < rabubanj::drumCount; ++d)
    {
        const auto i = static_cast<size_t>(d);
        const auto id = juce::String(d);
        gainParams[i] = parameters.getRawParameterValue("gain" + id);
        panParams[i] = parameters.getRawParameterValue("pan" + id);
        muteParams[i] = parameters.getRawParameterValue("mute" + id);
    }
    masterParam = parameters.getRawParameterValue("master");
    roomParam = parameters.getRawParameterValue("room");
}

void RaBubanjProcessor::prepareToPlay(double rate, int)
{
    engine.prepare(rate);
    reverb.setSampleRate(rate);
    reverb.reset();
    masterGain.reset(rate, .02);
    masterGain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(masterParam->load()));
    padFifo.reset();
    peak.store(0);
}

void RaBubanjProcessor::releaseResources()
{
    engine.reset();
    reverb.reset();
}

bool RaBubanjProcessor::isBusesLayoutSupported(const BusesLayout& layout) const
{
    return layout.getMainInputChannelSet().isDisabled()
        && layout.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void RaBubanjProcessor::audition(int drum) noexcept
{
    if (drum < 0 || drum >= rabubanj::drumCount) return;
    int start1, size1, start2, size2;
    padFifo.prepareToWrite(1, start1, size1, start2, size2);
    if (size1 > 0) padQueue[static_cast<size_t>(start1)] = drum;
    padFifo.finishedWrite(size1);
}

void RaBubanjProcessor::hit(int note, float velocity) noexcept
{
    const auto drum = rabubanj::drumForNote(note);
    if (drum < 0 || velocity <= 0.0f) return;
    engine.trigger(note, velocity);
    hits[static_cast<size_t>(drum)].fetch_add(1, std::memory_order_relaxed);
}

void RaBubanjProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    if (buffer.getNumChannels() < 2) { midi.clear(); return; }
    const int samples = buffer.getNumSamples();
    // Preserve queued preview hits until a nonempty host block arrives.
    if (samples == 0) { midi.clear(); return; }
    rabubanj::Mix mix;
    for (size_t i = 0; i < gainParams.size(); ++i)
    {
        mix.gain[i] = juce::Decibels::decibelsToGain(gainParams[i]->load());
        mix.pan[i] = panParams[i]->load();
        mix.mute[i] = muteParams[i]->load() >= .5f;
    }
    int start1, size1, start2, size2;
    padFifo.prepareToRead(padFifo.getNumReady(), start1, size1, start2, size2);
    for (int i = 0; i < size1; ++i) hit(rabubanj::notes[static_cast<size_t>(padQueue[static_cast<size_t>(start1 + i)])], .85f);
    for (int i = 0; i < size2; ++i) hit(rabubanj::notes[static_cast<size_t>(padQueue[static_cast<size_t>(start2 + i)])], .85f);
    padFifo.finishedRead(size1 + size2);

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    juce::Reverb::Parameters room;
    room.roomSize = .45f;
    room.damping = .55f;
    room.width = .9f;
    room.wetLevel = roomParam->load() * .3f;
    room.dryLevel = 1.0f;
    reverb.setParameters(room);
    masterGain.setTargetValue(juce::Decibels::decibelsToGain(masterParam->load()));
    float blockPeak = 0;
    auto renderSegment = [&](int offset, int length)
    {
        if (length <= 0) return;
        engine.render(left + offset, right + offset, length, mix);
        reverb.processStereo(left + offset, right + offset, length);
        for (int s = offset; s < offset + length; ++s)
        {
            const auto gain = masterGain.getNextValue();
            left[s] = std::tanh(left[s] * gain);
            right[s] = std::tanh(right[s] * gain);
            blockPeak = std::max(blockPeak, std::max(std::abs(left[s]), std::abs(right[s])));
        }
    };
    int cursor = 0;
    for (const auto metadata : midi)
    {
        const auto position = juce::jlimit(cursor, samples, metadata.samplePosition);
        renderSegment(cursor, position - cursor);
        cursor = position;
        // Ignore long system messages without allocating a MidiMessage on the audio thread.
        if (metadata.numBytes > 3) continue;
        const auto message = metadata.getMessage();
        if (message.isNoteOn()) hit(message.getNoteNumber(), message.getFloatVelocity());
        else if (message.isAllNotesOff() || message.isAllSoundOff()) { engine.reset(); reverb.reset(); }
    }
    renderSegment(cursor, samples - cursor);
    midi.clear();
    const auto decay = static_cast<float>(std::exp(-static_cast<double>(samples) / (getSampleRate() * .3)));
    peak.store(std::max(blockPeak, peak.load() * decay));
}

void RaBubanjProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    if (auto xml = parameters.copyState().createXml()) copyXmlToBinary(*xml, destination);
}

void RaBubanjProcessor::setStateInformation(const void* data, int size)
{
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* RaBubanjProcessor::createEditor() { return new RaBubanjEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RaBubanjProcessor(); }
