#include <spectr/audio_loader/AudioLoader.h>

#include <spectr/audio_loader/WavLoader.h>

#include <spectr/utils/Assert.h>
#include <spectr/utils/Asset.h>
#include <spectr/utils/Exception.h>

#include <algorithm>
#include <cwctype>
#include <fstream>
#include <locale>

namespace spectr::audio_loader
{
AudioData AudioLoader::load(const std::filesystem::path& audioPath)
{
    if (!std::filesystem::exists(audioPath))
    {
        throw utils::Exception("Specified file doesn't exist: {}", audioPath);
    }

    if (std::filesystem::file_size(audioPath) == 0)
    {
        throw utils::Exception("Specified file is empty (zero bytes): {}", audioPath);
    }

    const auto extensionWithDot = audioPath.extension().string();
    if (extensionWithDot.empty())
    {
        throw utils::Exception("Cannot recognize file type without extension.");
    }

    auto extension = extensionWithDot.substr(1);
    ASSERT(!extension.empty());

    std::transform(extension.begin(),
                   extension.end(),
                   extension.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (extension == "wav")
    {
        std::ifstream file{ audioPath, std::ios_base::binary };
        return WavLoader::load(file);
    }

    throw utils::Exception("Unsupported audio file extensions: {}", extension);
}

AudioData AudioLoader::loadAsset(const std::filesystem::path& audioAssetPathRelative)
{
    const auto audioPath = utils::Asset::getPath(audioAssetPathRelative);
    return load(audioPath);
}
}
