#include <spectr/audio_loader/AudioLoader.h>

#include <spectr/audio_loader/WavLoader.h>

#include <spectr/utils/Assert.h>
#include <spectr/utils/Exception.h>

#include <algorithm>
#include <fstream>

namespace spectr::audio_loader
{
SignalData AudioLoader::load(const std::filesystem::path& audioFilePath)
{
    if (!std::filesystem::exists(audioFilePath))
    {
        throw utils::Exception("Specified file doesn't exist: {}", audioFilePath);
    }

    if (std::filesystem::file_size(audioFilePath) == 0)
    {
        throw utils::Exception("Specified file is empty (zero bytes): {}", audioFilePath);
    }

    const auto extensionWithDot = audioFilePath.extension().string();
    if (extensionWithDot.empty())
    {
        throw utils::Exception("Cannot recognize the type of audio file without extension.");
    }

    auto extension = extensionWithDot.substr(1);
    ASSERT(!extension.empty());

    std::transform(extension.begin(),
                   extension.end(),
                   extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    std::ifstream file{ audioFilePath, std::ios_base::binary };

    if (extension == "wav")
    {
        return WavLoader::load(file);
    }

    throw utils::Exception("Unsupported audio file extension: {}", audioFilePath);
}
}
