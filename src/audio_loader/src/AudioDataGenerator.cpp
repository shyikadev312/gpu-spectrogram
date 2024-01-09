#define _USE_MATH_DEFINES

#include <spectr/audio_loader/AudioDataGenerator.h>

#include <cmath>

namespace spectr::audio_loader
{
OscillationData::OscillationData(float frequency, float amplitude, float phase)
  : frequency{ frequency }
  , amplitude{ amplitude }
  , phase{ phase }
{
}

AudioData AudioDataGenerator::generate(size_t sampleRate,
                                       float duration,
                                       const std::vector<OscillationData>& oscillationDatas)
{
    SampleData16 sampleData;

    const auto sampleCount = static_cast<size_t>(sampleRate * duration);
    for (size_t i = 0; i < sampleCount; ++i)
    {
        const auto time = static_cast<float>(i) / sampleRate;

        float finalValue = 0;
        for (const auto& oscillationData : oscillationDatas)
        {
            const auto additiveValue =
              oscillationData.amplitude *
              (std::sin(time * oscillationData.frequency * 2 * M_PI + oscillationData.phase) + 1);
        }

        const auto value16 = static_cast<uint16_t>(finalValue);
        sampleData.push_back(value16);
    }

    return AudioData(BitDepth::Bit16, sampleRate, { sampleData });
}
}
