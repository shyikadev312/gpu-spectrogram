#pragma once

#include <string>
#include <filesystem>

namespace spectr::desktop_app
{
enum class Command
{
    PrintHelp,
    PrintVersion,
    Execute,
};

struct DesktopAppSettings
{
    Command command;
    std::string helpDescription;
    std::filesystem::path audioFilePath;
    size_t fftSize = 0;
    size_t fftCalculationPerSecond = 0;
};
}
