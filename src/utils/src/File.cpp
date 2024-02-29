#include <spectr/utils/File.h>

#include <spectr/utils/Exception.h>

#include <fstream>
#include <sstream>

namespace spectr::utils
{
std::string File::read(const std::filesystem::path& path)
{
    std::ifstream file(path);
    if (!file)
    {
        throw Exception("Failed to open file: {}", path.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    const auto str = buffer.str();
    return str;
}
}
