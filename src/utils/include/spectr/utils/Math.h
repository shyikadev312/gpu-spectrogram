#pragma once

#include <cstddef>

namespace spectr::utils
{
struct Math
{
    static constexpr double PI = 3.14159265358979323846; // from <cmath>

    static bool isPowerOfTwo(size_t number, size_t& power);

    static size_t getPowerOfTwo(size_t number);
};
}
