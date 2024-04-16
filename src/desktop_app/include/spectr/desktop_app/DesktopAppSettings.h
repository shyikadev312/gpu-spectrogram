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

enum class BackendEngine {
    CPU,
    OpenCL,
    CUDA
};

enum class FrontendEngine {
    OpenGL
};

struct DesktopAppSettings
{
    Command command;
    std::string helpDescription;
    std::filesystem::path audioFilePath;
    size_t fftSize = 0;
    size_t fftCalculationPerSecond = 0;
    BackendEngine backend = BackendEngine::CUDA;
    FrontendEngine frontend = FrontendEngine::OpenGL;
};
}
