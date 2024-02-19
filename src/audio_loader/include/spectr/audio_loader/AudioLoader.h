#pragma once

#include <spectr/audio_loader/SignalData.h>

#include <filesystem>

namespace spectr::audio_loader
{
class AudioLoader
{
public:
    static SignalData load(const std::filesystem::path& audioFilePath);
};
}
