#if OS_LINUX

#include <spectr/utils/OsUtils.h>

#include <linux/limits.h>
#include <unistd.h>

#include <array>
#include <stdexcept>

namespace
{
std::filesystem::path getExecutablePath()
{
    std::array<char, PATH_MAX> buffer{};
    const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size());
    if (length <= 0 || length == buffer.size())
    {
        throw std::runtime_error("Failed to get executable path.");
    }
    std::filesystem::path path{ buffer.begin(), buffer.begin() + length };
    return path.make_preferred();
}
}

namespace spectr::utils
{
Os getOs()
{
    return Os::Linux;
}

const std::filesystem::path& getExecutablePath()
{
    static const auto executablePath = ::getExecutablePath();
    return executablePath;
}
}
#endif
