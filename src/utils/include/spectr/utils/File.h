#pragma once

#include <filesystem>
#include <string>

namespace spectr::utils
{
class File
{
public:
    static std::string read(const std::filesystem::path& path);
};
}
