#pragma once

#include <spectr/desktop_app/DesktopAppSettings.h>

namespace spectr::desktop_app
{
class CmdArgumentParser
{
public:
    static DesktopAppSettings parse(int argc, const char* argv[]);
};
}
