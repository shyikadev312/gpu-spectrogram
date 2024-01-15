#include <spectr/utils/Timer.h>

#include <iomanip>
#include <sstream>

namespace spectr::utils
{
Timer::Timer()
  : m_start{ std::chrono::high_resolution_clock::now() }
{
}

float Timer::getTime() const
{
    const auto now = std::chrono::high_resolution_clock::now();
    const auto diff = now - m_start;
    const std::chrono::duration<float> diffSeconds = diff;
    return diffSeconds.count();
}

float Timer::getMs() const
{
    return getTime() * 1000.0f;
}

void Timer::restart()
{
    m_start = std::chrono::high_resolution_clock::now();
}

std::string Timer::toString() const
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << getMs() << " ms.";
    return ss.str();
}
}
