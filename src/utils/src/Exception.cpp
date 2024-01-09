#include <spectr/utils/Exception.h>

namespace spectr::utils
{
Exception::Exception()
  : std::runtime_error("")
{
}

Exception::Exception(const char* string)
  : std::runtime_error(string)
{
}

Exception::Exception(const std::string& str)
  : std::runtime_error(str)
{
}
}
