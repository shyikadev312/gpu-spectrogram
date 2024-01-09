#include <spectr/utils/Asset.h>

#include <spectr/utils/Assert.h>
#include <spectr/utils/OsUtils.h>

namespace spectr::utils
{
std::filesystem::path getAssetDir()
{
    const auto exePath = utils::getExecutablePath();
    ASSERT(exePath.is_absolute());
    const auto assetDir = exePath.parent_path().append("assets");
    return assetDir;
}

std::filesystem::path Asset::getPath(const std::filesystem::path& assetRelativePath)
{
    ASSERT(!assetRelativePath.is_absolute());

    const auto assetDir = getAssetDir();
    ASSERT(assetDir.is_absolute());

    // uncomment to allow passing of already resolved asset paths
    /*if (assetRelativePath.is_absolute())
    {
        ASSERT(assetRelativePath.string().starts_with(assetDir.string()));
        return snippetRelativePath;
    }*/

    const auto assetPath = (assetDir / assetRelativePath).make_preferred();
    return assetPath;
}
}
