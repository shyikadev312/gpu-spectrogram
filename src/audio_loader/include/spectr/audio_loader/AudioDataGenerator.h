#pragma once

#include <spectr/audio_loader/AudioData.h>

#include <vector>

namespace spectr::audio_loader
{
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
    static AudioData generate(size_t sampleRate,
                              float duration,
                              const std::vector<OscillationData>& oscillationDatas);
};
}
