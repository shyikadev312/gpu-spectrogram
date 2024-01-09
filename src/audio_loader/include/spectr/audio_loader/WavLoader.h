#pragma once

#include <spectr/audio_loader/AudioData.h>

#include <filesystem>
#include <iostream>

namespace spectr::audio_loader
{
class WavLoader
{
public:
    static AudioData load(std::istream& reader);
};
}
