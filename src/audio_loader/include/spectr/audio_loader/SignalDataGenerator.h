#pragma once

#include <spectr/audio_loader/SignalData.h>

#include <cmath>
#include <vector>

namespace spectr::audio_loader
{
namespace
{
constexpr auto PI = 3.14159265358979323846;
}

/**
 * @brief Data for specifying a one sine wave.
 */
class SineWaveInfo
{
public:
    SineWaveInfo(float frequency, float amplitude = 1, float phase = 0);

    float frequency;
    float amplitude = 1;
    float phase = 0;
};

/**
 * @brief Generates signals from parameters.
 */
class SignalDataGenerator
{
public:
    /**
     * @brief Generates a resulting (combined) signal from specified settings of several sine waves.
     */
    template<typename T>
    static SignalData generate(size_t sampleRate,
                               float duration,
                               const std::vector<SineWaveInfo>& sineWaveInfos);

    /**
     * @brief Generate signal filled with one constant value.
     */
    template<typename T>
    static SignalData generateWithConstant(size_t sampleRate, float duration, T constantValue);
};

// Template methods definitions:

template<typename T>
SignalData SignalDataGenerator::generate(size_t sampleRate,
                                         float duration,
                                         const std::vector<SineWaveInfo>& sineWaveInfos)
{
    SampleData<T> sampleData;

    const auto sampleCount = static_cast<size_t>(sampleRate * duration);
    for (size_t i = 0; i < sampleCount; ++i)
    {
        const auto time = static_cast<double>(i) / sampleRate;

        double finalValue = 0;
        for (const auto& sineWaveInfo : sineWaveInfos)
        {
            const auto additiveValue =
              sineWaveInfo.amplitude *
              (std::sin(time * sineWaveInfo.frequency * 2 * PI + sineWaveInfo.phase));

            finalValue += additiveValue;
        }

        const auto valueT = static_cast<T>(finalValue);
        sampleData.push_back(valueT);
    }

    return SignalData(sampleRate, { std::move(sampleData) });
}

template<typename T>
SignalData SignalDataGenerator::generateWithConstant(size_t sampleRate,
                                                            float duration,
                                                            T constantValue)
{
    SampleData<T> sampleData;
    const auto sampleCount = static_cast<size_t>(sampleRate * duration);
    for (size_t i = 0; i < sampleCount; ++i)
    {
        sampleData.push_back(constantValue);
    }
    return SignalData(sampleRate, { std::move(sampleData) });
}
}
