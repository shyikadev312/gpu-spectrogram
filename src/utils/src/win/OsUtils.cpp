#ifdef _WIN32

#include <spectr/utils/Assert.h>
#include <spectr/utils/OsUtils.h>

#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX 1
#include <Windows.h>

#include <array>
#include <stdexcept>

namespace
{
std::filesystem::path getExecutablePath()
{
    return std::filesystem::current_path();

    // why would anyone ever do this
    /*
    size_t pathLength = 0;
    std::vector<char> buffer(MAX_PATH, '\0');
    while (true)
    {
        pathLength = GetModuleFileNameA(NULL, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (pathLength == 0)
        {
            throw std::runtime_error("Failed to get executable path.");
        }

        if (pathLength < buffer.size())
        {
            break;
        }

        ASSERT(GetLastError() == ERROR_INSUFFICIENT_BUFFER);
        buffer.resize(buffer.size() * 2, '\0');
    }
    return std::filesystem::path{ buffer.data(), buffer.data() + pathLength };
    */
}
}

namespace spectr::utils
{
Os getOs()
{
    return Os::Windows;
}

const std::filesystem::path& getExecutablePath()
{
    static const auto executablePath = ::getExecutablePath();
    return executablePath;
}
}

#endif
