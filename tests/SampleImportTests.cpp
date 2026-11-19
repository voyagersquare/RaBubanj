#include "PluginEditor.h"
#include <cstdlib>
#include <iostream>
#include <thread>
static void require(bool ok, const char* message)
{ if (!ok) { std::cerr << "FAIL: " << message << '\n'; std::exit(1); } }
static void waitFor(RaBubanjProcessor& p, int drum, SampleLoader::Status status)
{
    const auto start = juce::Time::getMillisecondCounterHiRes();
    while (p.getSampleInfo(drum).status != status && juce::Time::getMillisecondCounterHiRes()-start < 6000) juce::Thread::sleep(5);
    require(p.getSampleInfo(drum).status == status, "asynchronous sample request must reach the expected status");
}
static void writeSample(const juce::File& file, bool stereo, int length = 16000)
{
    juce::WavAudioFormat format; auto output = file.createOutputStream(); require(output != nullptr, "temporary WAV must open");
    std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(output.release(), 32000, stereo ? 2u : 1u, 16, {}, 0));
    require(writer != nullptr, "WAV writer must open");
    juce::AudioBuffer<float> data(stereo ? 2 : 1, length);
    for (int i = 0; i < length; ++i)
    {
        const auto t = static_cast<double>(i)/32000, env = std::min(1.0, t*2000)*std::exp(-t*12);
        data.setSample(0, i, static_cast<float>(.4*std::sin(t*juce::MathConstants<double>::twoPi*193)*env));
        if (stereo) data.setSample(1, i, static_cast<float>(.2*std::sin(t*juce::MathConstants<double>::twoPi*317)*env));
    }
    require(writer->writeFromAudioSampleBuffer(data, 0, length), "test WAV must be written");
}
static void prepare(RaBubanjProcessor& p) { p.setRateAndBufferSizeDetails(48000, 512); p.prepareToPlay(48000, 512); }
static double preview(RaBubanjProcessor& p, int drum, bool stereo = false)
{
    p.releaseResources(); prepare(p); juce::AudioBuffer<float> audio(2, 512); juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, 36+drum, .9f), 0);
    double energy = 0, difference = 0;
    for (int block = 0; block < 80; ++block)
    {
        p.processBlock(audio, midi);
        for (int s = 0; s < 512; ++s)
        {
            const auto l = audio.getSample(0, s), r = audio.getSample(1, s);
            require(std::isfinite(l) && std::isfinite(r), "imported audio must remain finite");
            energy += l*l+r*r; difference += (l-r)*(l-r);
        }
    }
    if (stereo) require(difference > 1, "stereo samples must preserve distinct left/right content");
    return energy;
}
struct RecordingPlayHead : juce::AudioPlayHead
{
    juce::Optional<PositionInfo> getPosition() const override { PositionInfo info; info.setIsRecording(true); return info; }
};
int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    const auto directory = juce::File::getSpecialLocation(juce::File::tempDirectory).getNonexistentChildFile("RaBubanj-sample-test", "", false);
    require(directory.createDirectory().wasOk(), "temporary sample directory must exist");
    const auto mono = directory.getChildFile("kick sample.wav"), stereo = directory.getChildFile("stereo snare.wav"), bad = directory.getChildFile("bad.wav");
    writeSample(mono, false); writeSample(stereo, true); require(bad.replaceWithText("not audio"), "malformed fixture must write");
    RaBubanjProcessor p; prepare(p);
    std::unique_ptr<RaBubanjEditor> editor(static_cast<RaBubanjEditor*>(p.createEditor()));
    MixerStrip* kick = nullptr; MixerStrip* snare = nullptr; MixerStrip* reverb = nullptr;
    for (auto* child : editor->getChildren()) if (auto* strip = dynamic_cast<MixerStrip*>(child))
    { if (strip->getName() == "PAD 1 channel") kick = strip; if (strip->getName() == "PAD 2 channel") snare = strip; if (strip->getName() == "Reverb channel") reverb = strip; }
    require(kick && snare && reverb, "drop targets must exist");
    const juce::StringArray paths { mono.getFullPathName() };
    require(kick->isInterestedInFileDrag(paths) && !reverb->isInterestedInFileDrag(paths), "only drum strips accept sample drops");
    require(!kick->isInterestedInFileDrag({ mono.getFullPathName(), stereo.getFullPathName() }) && !kick->isInterestedInTextDrag("proprietary clip payload"), "ambiguous and non-file payloads must be rejected");
    kick->filesDropped(paths, 10, 10); waitFor(p, 0, SampleLoader::Status::Ready);
    const auto fileUrl = juce::URL(stereo).toString(false);
    require(snare->isInterestedInTextDrag(fileUrl), "file URI drops must support paths containing spaces");
    snare->textDropped(fileUrl, 10, 10); waitFor(p, 1, SampleLoader::Status::Ready);
    require(p.getSampleInfo(0).name == mono.getFileName() && p.getSampleInfo(1).name == stereo.getFileName()
        && !p.getSampleInfo(2).active, "each drop must replace only its target channel");
    const auto monoEnergy = preview(p, 0), stereoEnergy = preview(p, 1, true);
    require(monoEnergy > 1 && stereoEnergy > 1, "32 kHz imports must play at a 48 kHz host rate");
    p.importSample(0, bad); waitFor(p, 0, SampleLoader::Status::Error);
    require(p.getSampleInfo(0).active && std::abs(preview(p, 0)-monoEnergy) < .000001, "failed import must preserve the existing source");
    const auto longFile = directory.getChildFile("too-long.wav"), largeFile = directory.getChildFile("too-large.wav");
    writeSample(longFile, false, 32000*31);
    p.importSample(0, longFile); waitFor(p, 0, SampleLoader::Status::Error);
    { auto stream = largeFile.createOutputStream(); require(stream != nullptr && stream->setPosition(32*1024*1024+1) && stream->writeByte(0), "large fixture must write"); }
    p.importSample(0, largeFile); waitFor(p, 0, SampleLoader::Status::Error);
    require(p.getSampleInfo(0).active && std::abs(preview(p, 0)-monoEnergy) < .000001, "oversize and overlong imports must preserve existing audio");
    p.setLocalKeys(true);
    juce::MemoryBlock state; p.getStateInformation(state);
    require(mono.deleteFile() && stereo.deleteFile(), "original files must be removable before recall");
    RaBubanjProcessor restored; restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    waitFor(restored, 0, SampleLoader::Status::Ready); waitFor(restored, 1, SampleLoader::Status::Ready);
    require(restored.localKeysEnabled(), "keyboard mode must recall");
    require(std::abs(preview(restored, 0)-monoEnergy) < .000001 && std::abs(preview(restored, 1, true)-stereoEnergy) < .000001,
        "embedded samples must recall without their original file paths");
    restored.clearSample(0);
    require(!restored.getSampleInfo(0).active && restored.getSampleInfo(1).active, "clear must remove only its own imported source");
    restored.importSample(0, mono); waitFor(restored, 0, SampleLoader::Status::Error);
    require(!restored.getSampleInfo(0).active, "missing new file must leave factory source active");
    // Corrupt/missing restored content must remain explicitly missing, not play a different factory sound.
    auto tree = juce::ValueTree::fromXml(*juce::AudioProcessor::getXmlFromBinary(state.getData(), static_cast<int>(state.getSize())));
    tree.getChildWithName("SAMPLES").getChild(0).setProperty("data", "", nullptr);
    juce::AudioProcessor::copyXmlToBinary(*tree.createXml(), state);
    restored.setStateInformation(state.getData(), static_cast<int>(state.getSize())); waitFor(restored, 0, SampleLoader::Status::Missing);
    require(preview(restored, 0) == 0, "missing recalled sample must be silent until repaired or cleared");
    writeSample(mono, false); writeSample(stereo, true);
    // Exercise publication, tail references, voice stealing and stale request cancellation while audio runs.
    prepare(p); std::atomic<bool> running { true };
    std::thread audioThread([&]
    {
        juce::AudioBuffer<float> audio(2, 512); juce::MidiBuffer midi;
        while (running.load())
        {
            for (int i = 0; i < 8; ++i) midi.addEvent(juce::MidiMessage::noteOn(1, 36+i, .7f), 0);
            p.processBlock(audio, midi);
            for (int c = 0; c < 2; ++c) for (int i = 0; i < 512; ++i) require(std::isfinite(audio.getSample(c, i)), "concurrent import audio must stay finite");
        }
    });
    for (int i = 0; i < 80; ++i)
    {
        p.importSample(i%8, i%2 ? mono : stereo);
        if (i%3 == 0) p.clearSample(i%8);
        juce::Thread::sleep(3);
    }
    running.store(false); audioThread.join(); p.releaseResources();
    for (int d = 0; d < 8; ++d) p.clearSample(d);
    for (int i = 0; i < 25; ++i) p.importSample(0, stereo);
    p.clearSample(0); juce::Thread::sleep(150);
    require(!p.getSampleInfo(0).active && p.getSampleInfo(0).status == SampleLoader::Status::Empty, "clear must invalidate every pending import");
    RecordingPlayHead head; p.setPlayHead(&head); p.setLocalKeys(true);
    juce::AudioBuffer<float> buffer(2, 512); juce::MidiBuffer midi; p.processBlock(buffer, midi);
    require(!editor->keyPressed(juce::KeyPress('A')), "local audition must forward keys while the DAW is recording"); p.setPlayHead(nullptr);
    require(directory.deleteRecursively(), "temporary sample fixtures must clean up");
    std::cout << "Per-strip WAV/URI drops, stereo/resampling, failure handling, embedded recall, live swaps, cancellation and recording keyboard forwarding passed.\n";
}
