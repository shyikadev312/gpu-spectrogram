#include <spectr/utils/Assert.h>

#include <iostream>

namespace spectr::utils
{
void assertWithMessage(const char* exprStr,
                       bool isExprValid,
                       const char* file,
                       int line,
                       const char* message)
{
#if !defined(NDEBUG) or defined(FORCE_ENABLE_ASSERTIONS)
    if (!isExprValid)
    {
        std::cerr << "Assertion failed: " << message << "\n"
                  << "Expected: " << exprStr << "\n"
                  << "Source: " << file << ", line #" << line << std::endl;
        std::abort();
    }
#endif
}

void assertWithoutMessage(const char* exprStr, bool isExprValid, const char* file, int line)
{
#if !defined(NDEBUG) or defined(FORCE_ENABLE_ASSERTIONS)
    if (!isExprValid)
    {
        std::cerr << "Assertion failed: " << exprStr << "\n"
                  << "Source: " << file << ", line #" << line << std::endl;
        std::abort();
    }
#endif
}
}
