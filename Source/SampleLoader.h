#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "CustomSample.h"
#include <array>
#include <atomic>
#include <deque>
#include <mutex>

class SampleLoader final : private juce::Thread
{
public:
    enum class Status { Empty, Loading, Ready, Missing, Error };
    struct Info { Status status = Status::Empty; juce::String name, path, message; bool active = false; };
    struct Snapshot { std::array<rabubanj::CustomSample*, 8> sources {}; };
    SampleLoader();
    ~SampleLoader() override;
    void shutdown();
    void load(int drum, const juce::File&);
    void clear(int drum, bool notifyHost = true);
    Info getInfo(int drum) const;
    bool usesCustom(int drum) const noexcept { return wanted[static_cast<size_t>(drum)].load(); }
    const Snapshot* beginAudioRead() noexcept;
    void endAudioRead() noexcept { hazard.store(nullptr); }
    rabubanj::CustomSample* audioSource(const Snapshot&, int drum) const noexcept;
    juce::ValueTree save() const;
    void restore(const juce::ValueTree&);
    static juce::File dropFile(const juce::StringArray&);
    std::function<void()> onChange;
private:
    struct Recall { bool wanted = false; juce::String name, path, data; };
    struct Request { int drum; unsigned generation; juce::String name, path, data; bool restoring = false, clearing = false, notify = true; };
    void enqueue(Request);
    void run() override;
    void process(const Request&);
    void publish(int drum, rabubanj::CustomSample*);
    void collect();
    juce::AudioFormatManager formats;
    mutable std::mutex infoMutex;
    std::mutex queueMutex;
    std::array<Info, 8> infos;
    std::array<Recall, 8> recalls;
    std::array<std::atomic<unsigned>, 8> generations {}, minimumGeneration {};
    std::array<std::atomic<bool>, 8> wanted {};
    std::deque<Request> requests;
    std::vector<std::unique_ptr<rabubanj::CustomSample>> samples;
    std::vector<std::unique_ptr<Snapshot>> snapshots;
    std::atomic<Snapshot*> published { nullptr };
    std::atomic<const Snapshot*> hazard { nullptr };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleLoader)
};
