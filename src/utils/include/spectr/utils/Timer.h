#pragma once

#include <chrono>
#include <string>

namespace spectr::utils
{
class Timer
{
public:
    Timer();

    /**
     * @brief Get passed time in seconds since the timer creation.
     * @return Time in seconds.
     */
    float getTime() const;

    /**
     * @brief Get passed time in milliseconds since the timer creation.
     * @return Time in milliseconds.
     */
    float getMs() const;

    void restart();

    std::string toString() const;

private:
    std::chrono::high_resolution_clock::time_point m_start;
};
}
