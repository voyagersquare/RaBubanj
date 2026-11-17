#pragma once
#include <atomic>
#include <vector>
namespace rabubanj
{
struct CustomSample
{
    std::vector<float> left, right;
    double sampleRate = 44100;
    unsigned generation = 0;
    std::atomic<unsigned> voiceReferences { 0 };
};
}
