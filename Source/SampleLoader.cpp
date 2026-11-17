#include "SampleLoader.h"
#include <cmath>
SampleLoader::SampleLoader() : Thread("RaBubanj sample loader")
{
    formats.registerBasicFormats(); snapshots.push_back(std::make_unique<Snapshot>());
    published.store(snapshots.back().get()); startThread();
}
SampleLoader::~SampleLoader() { shutdown(); }
void SampleLoader::shutdown() { signalThreadShouldExit(); notify(); stopThread(-1); }
juce::File SampleLoader::dropFile(const juce::StringArray& paths)
{
    if (paths.size() != 1) return {};
    const auto path = paths[0].trim().unquoted(); juce::File file;
    if (path.startsWithIgnoreCase("file://")) { const juce::URL url(path); if (url.isLocalFile()) file = url.getLocalFile(); }
    else if (juce::File::isAbsolutePath(path)) file = juce::File(path);
    return file.hasFileExtension("wav;aif;aiff;flac;ogg") ? file : juce::File();
}
void SampleLoader::enqueue(Request request)
{
    { std::lock_guard<std::mutex> lock(queueMutex); requests.push_back(std::move(request)); }
    notify();
}
void SampleLoader::load(int drum, const juce::File& file)
{
    if (drum < 0 || drum >= 8) return;
    const auto i = static_cast<size_t>(drum); unsigned gen;
    { std::lock_guard<std::mutex> lock(infoMutex); gen = generations[i].fetch_add(1)+1;
      infos[i].status = Status::Loading; infos[i].message = "Loading "+file.getFileName(); }
    enqueue({ drum, static_cast<unsigned>(gen), file.getFileName(), file.getFullPathName(), {}, false, false, true });
}
void SampleLoader::clear(int drum, bool notifyHost)
{
    if (drum < 0 || drum >= 8) return;
    const auto i = static_cast<size_t>(drum); unsigned gen;
    { std::lock_guard<std::mutex> lock(infoMutex); gen = generations[i].fetch_add(1)+1;
      minimumGeneration[i].store(gen); wanted[i].store(false); infos[i] = {}; recalls[i] = {}; }
    enqueue({ drum, gen, {}, {}, {}, false, true, notifyHost });
}
SampleLoader::Info SampleLoader::getInfo(int drum) const
{
    std::lock_guard<std::mutex> lock(infoMutex); auto result = infos[static_cast<size_t>(drum)];
    result.active = wanted[static_cast<size_t>(drum)].load(); return result;
}
const SampleLoader::Snapshot* SampleLoader::beginAudioRead() noexcept
{
    const Snapshot* snapshot;
    // One audio reader holds this snapshot for the entire block. Only the worker reclaims it.
    do { snapshot = published.load(); hazard.store(snapshot); } while (snapshot != published.load());
    return snapshot;
}
rabubanj::CustomSample* SampleLoader::audioSource(const Snapshot& snapshot, int drum) const noexcept
{
    const auto i = static_cast<size_t>(drum); auto* sample = snapshot.sources[i];
    return sample != nullptr && sample->generation >= minimumGeneration[i].load() ? sample : nullptr;
}
void SampleLoader::publish(int drum, rabubanj::CustomSample* sample)
{
    auto snapshot = std::make_unique<Snapshot>(*published.load()); snapshot->sources[static_cast<size_t>(drum)] = sample;
    auto* pointer = snapshot.get(); snapshots.push_back(std::move(snapshot)); published.store(pointer);
}
void SampleLoader::collect()
{
    const auto* live = published.load(); const auto* protectedAddress = hazard.load();
    // A reader may briefly publish an already-retired address while validating its hazard.
    // Compare addresses first; never dereference a hazard that is no longer owned here.
    const Snapshot* reading = nullptr;
    for (const auto& snapshot : snapshots) if (snapshot.get() == protectedAddress) { reading = snapshot.get(); break; }
    snapshots.erase(std::remove_if(snapshots.begin(), snapshots.end(), [&](const auto& snapshot)
        { return snapshot.get() != live && snapshot.get() != reading; }), snapshots.end());
    auto contains = [](const Snapshot* set, const rabubanj::CustomSample* sample)
        { return set && std::find(set->sources.begin(), set->sources.end(), sample) != set->sources.end(); };
    samples.erase(std::remove_if(samples.begin(), samples.end(), [&](const auto& sample)
        { return !contains(live, sample.get()) && !contains(reading, sample.get()) && sample->voiceReferences.load() == 0; }), samples.end());
}
void SampleLoader::run()
{
    while (!threadShouldExit())
    {
        std::unique_ptr<Request> request;
        { std::lock_guard<std::mutex> lock(queueMutex); if (!requests.empty()) { request = std::make_unique<Request>(std::move(requests.front())); requests.pop_front(); } }
        if (request) process(*request);
        collect(); if (!request) wait(50);
    }
}
void SampleLoader::process(const Request& request)
{
    const auto i = static_cast<size_t>(request.drum);
    if (request.generation != generations[i].load()) return;
    if (request.clearing)
    {
        { std::lock_guard<std::mutex> lock(infoMutex);
          if (request.generation != generations[i].load()) return;
          publish(request.drum, nullptr); }
        if (request.notify && onChange) onChange();
        return;
    }
    juce::String error;
    juce::MemoryBlock bytes;
    if (request.data.isNotEmpty())
    {
        if (request.data.length() > 48*1024*1024 || !bytes.fromBase64Encoding(request.data)) error = "Invalid embedded sample";
    }
    else
    {
        const juce::File file(request.path);
        if (!file.existsAsFile()) error = "Sample file is missing";
        else if (file.getSize() > 32*1024*1024) error = "Sample file exceeds 32 MB";
        else if (!file.loadFileAsData(bytes)) error = "Sample file could not be read";
    }
    if (bytes.getSize() > 32u*1024u*1024u) error = "Sample file exceeds 32 MB";
    std::unique_ptr<juce::AudioFormatReader> reader;
    if (error.isEmpty()) reader.reset(formats.createReaderFor(std::make_unique<juce::MemoryInputStream>(bytes, false)));
    if (error.isEmpty() && !reader) error = "Unsupported or invalid audio file";
    if (reader && (reader->sampleRate < 8000 || reader->sampleRate > 384000 || reader->numChannels < 1 || reader->numChannels > 2
        || reader->lengthInSamples < 2 || reader->lengthInSamples > reader->sampleRate*30
        || reader->lengthInSamples*reader->numChannels*static_cast<juce::int64>(sizeof(float)) > 32*1024*1024))
        error = "Use mono/stereo audio up to 30 seconds and 32 MB decoded";
    auto sample = std::make_unique<rabubanj::CustomSample>();
    if (error.isEmpty())
    {
        const auto length = static_cast<int>(reader->lengthInSamples);
        juce::AudioBuffer<float> decoded(static_cast<int>(reader->numChannels), length);
        if (!reader->read(&decoded, 0, length, 0, true, true)) error = "Audio decoding failed";
        else
        {
            sample->sampleRate = reader->sampleRate; sample->generation = request.generation;
            sample->left.assign(decoded.getReadPointer(0), decoded.getReadPointer(0)+length);
            if (reader->numChannels == 2) sample->right.assign(decoded.getReadPointer(1), decoded.getReadPointer(1)+length);
            for (auto* channel : { &sample->left, &sample->right }) for (auto& value : *channel)
                value = std::isfinite(value) ? std::clamp(value, -4.0f, 4.0f) : 0.0f;
        }
    }
    const auto encoded = error.isEmpty() ? (request.data.isNotEmpty() ? request.data : bytes.toBase64Encoding()) : juce::String();
    {
        std::lock_guard<std::mutex> lock(infoMutex);
        if (request.generation != generations[i].load()) return;
        if (error.isNotEmpty())
        {
            auto& info = infos[i]; info.status = request.restoring ? Status::Missing : Status::Error;
            info.message = error+": "+request.name; return;
        }
        auto* pointer = sample.get(); samples.push_back(std::move(sample)); publish(request.drum, pointer);
        wanted[i].store(true);
        infos[i] = { Status::Ready, request.name, request.path, "Loaded "+request.name, true };
        recalls[i] = { true, request.name, request.path, encoded };
    }
    if (request.notify && onChange) onChange();
}
juce::ValueTree SampleLoader::save() const
{
    juce::ValueTree result("SAMPLES"); std::lock_guard<std::mutex> lock(infoMutex);
    for (int d = 0; d < 8; ++d)
    {
        const auto& recall = recalls[static_cast<size_t>(d)]; if (!recall.wanted) continue;
        juce::ValueTree sample("SAMPLE"); sample.setProperty("drum", d, nullptr); sample.setProperty("name", recall.name, nullptr);
        sample.setProperty("path", recall.path, nullptr); sample.setProperty("data", recall.data, nullptr); result.addChild(sample, -1, nullptr);
    }
    return result;
}
void SampleLoader::restore(const juce::ValueTree& state)
{
    for (int drum = 0; drum < 8; ++drum) clear(drum, false);
    for (const auto& node : state)
    {
        const auto drum = static_cast<int>(node.getProperty("drum", -1)); if (drum < 0 || drum >= 8) continue;
        const auto i = static_cast<size_t>(drum); unsigned gen;
        const auto name = node.getProperty("name").toString(), path = node.getProperty("path").toString(), data = node.getProperty("data").toString();
        { std::lock_guard<std::mutex> lock(infoMutex); gen = generations[i].fetch_add(1)+1;
          minimumGeneration[i].store(gen); wanted[i].store(true); infos[i] = { Status::Loading, name, path, "Restoring "+name, true }; recalls[i] = { true, name, path, data }; }
        enqueue({ drum, gen, name, path, data, true, false, false });
    }
}
