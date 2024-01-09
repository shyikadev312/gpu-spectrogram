#pragma once

namespace spectr::utils
{

void assertWithMessage(const char* exprStr,
                       bool isExprValid,
                       const char* file,
                       int line,
                       const char* message);

void assertWithoutMessage(const char* exprStr, bool isExprValid, const char* file, int line);
}

#define ASSERT_MESSAGE(expr, message)                                                              \
    ::spectr::utils::assertWithMessage(#expr, (expr), __FILE__, __LINE__, message)

#define ASSERT(expr) ::spectr::utils::assertWithoutMessage(#expr, (expr), __FILE__, __LINE__)
