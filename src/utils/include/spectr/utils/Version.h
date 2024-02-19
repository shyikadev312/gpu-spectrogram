#pragma once

#include <iostream>

namespace spectr::utils
{
struct Version
{
    static void print(std::ostream& out, const std::string& toolName);
};
}
