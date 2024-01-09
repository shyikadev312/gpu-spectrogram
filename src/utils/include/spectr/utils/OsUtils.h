#pragma once

#include <filesystem>
#include <string>

namespace spectr::utils
{
enum class Os
{
    Undefined,
    Windows,
    Linux,
};

Os getOs();

const std::filesystem::path& getExecutablePath();
}
