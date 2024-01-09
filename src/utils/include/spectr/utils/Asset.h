#pragma once

#include <filesystem>

namespace spectr::utils
{
class Asset
{
public:
    static std::filesystem::path getPath(const std::filesystem::path& assetRelavetivePath);
};
}
