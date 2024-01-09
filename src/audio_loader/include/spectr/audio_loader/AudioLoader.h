#pragma once

#include <spectr/audio_loader/AudioData.h>

#include <filesystem>

namespace spectr::audio_loader
{
class AudioLoader
{
public:
    static AudioData load(const std::filesystem::path& audioPath);

    static AudioData loadAsset(const std::filesystem::path& audioAssetPathRelative);
};
}
