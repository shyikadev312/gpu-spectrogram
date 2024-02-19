#include <spectr/utils/Math.h>

#include <spectr/utils/Exception.h>

namespace spectr::utils
{
bool Math::isPowerOfTwo(size_t number, size_t& power)
{
    if (number == 0)
    {
        return false;
    }

    power = 0;
    while (number % 2 == 0)
    {
        power++;
        number /= 2;
    }
    return number == 1;
}

size_t Math::getPowerOfTwo(size_t number)
{
    size_t powerOfTwo;
    if (!isPowerOfTwo(number, powerOfTwo))
    {
        throw utils::Exception("Not power of 2: {}", number);
    }
    return powerOfTwo;
}
}
