#pragma once

#include <stdexcept>
#include <string>

#include <fmt/format.h>
#include <fmt/std.h>

namespace spectr::utils
{
class Exception : public std::runtime_error
{
public:
    Exception();

    Exception(const char* string);

    Exception(const std::string& str);

    template<typename... Args>
    Exception(fmt::format_string<Args...> formatString, Args&&... args);
};

template<typename... Args>
Exception::Exception(fmt::format_string<Args...> formatString, Args&&... args)
  : std::runtime_error(fmt::vformat(formatString, fmt::make_format_args(args...)))
{
}
}
