#pragma once

#include <stdexcept>
#include <string>

#include <format>

namespace spectr::utils
{
class Exception : public std::runtime_error
{
public:
    Exception();

    Exception(const char* string);

    Exception(const std::string& str);

    template<typename... Args>
    Exception(std::string formatString, Args&&... args);
};

template<typename... Args>
Exception::Exception(std::string formatString, Args&&... args)
  : std::runtime_error(std::vformat(formatString, std::make_format_args(args...)))
{
}
}
