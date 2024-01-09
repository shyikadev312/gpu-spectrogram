#pragma once

#include <vector>

namespace spectr::calc_cpu
{
/**
 * @brief Implementation of the Cooley–Tukey Radix-2 FFT algorithm.
 */
class FftCooleyTukeyRadix2
{
public:
    static std::vector<double> execute(const std::vector<double>& realValues);
};
}
