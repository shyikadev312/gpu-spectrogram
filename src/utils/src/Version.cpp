#include <spectr/utils/Version.h>

#include <spectr/utils/VersionConstants.h>

namespace spectr::utils
{
namespace
{
void printIsDebugBuild(std::ostream& out)
{
#ifdef _DEBUG
    out << "Debug build" << std::endl;
#else
    out << "Release build" << std::endl;
#endif
}

void printBuildDateTime(std::ostream& out)
{
    out << "Build date: " << __DATE__ << ", " << __TIME__ << std::endl;
}
}

void Version::print(std::ostream& out, const std::string& toolName)
{
    out << toolName << std::endl;
    out << "Version: " << SPECTR_VERSION << std::endl;
    out << "Commit hash: " << BUILD_COMMIT_HASH << std::endl;
    printBuildDateTime(out);
    printIsDebugBuild(out);
}
}
