#pragma once

#include <spectr/audio_loader/AudioData.h>

#include <cmath>
#include <vector>

namespace spectr::audio_loader
{
namespace
{
constexpr auto PI = 3.14159265358979323846;
}

class OscillationData
{
public:
    OscillationData(float frequency, float amplitude = 1, float phase = 0);

    float frequency;
    float amplitude = 1;
    float phase = 0;
};

class AudioDataGenerator
{
public:
    template<typename T>
    static AudioData generate(size_t sampleRate,
                              float duration,
                              const std::vector<OscillationData>& oscillationDatas);
};

template<typename T>
AudioData AudioDataGenerator::generate(size_t sampleRate,
                                       float duration,
                                       const std::vector<OscillationData>& oscillationDatas)
{
    SampleData<T> sampleData;

    const auto sampleCount = static_cast<size_t>(sampleRate * duration);
    for (size_t i = 0; i < sampleCount; ++i)
    {
        const auto time = static_cast<double>(i) / sampleRate;

        double finalValue = 0;
        for (const auto& oscillationData : oscillationDatas)
        {
            const auto additiveValue =
              oscillationData.amplitude *
              (std::sin(time * oscillationData.frequency * 2 * PI + oscillationData.phase));

            finalValue += additiveValue;
        }

        const auto valueT = static_cast<T>(finalValue);
        sampleData.push_back(valueT);
    }

    return AudioData(sampleRate, { sampleData });
}
}
