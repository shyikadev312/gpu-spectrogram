#pragma once

#include <cmath>
#include <complex>
#include <vector>

namespace spectr::calc_cpu
{
/**
 * @brief Implementation of the Cooley–Tukey Radix-2 FFT algorithm.
 */
class FftCooleyTukeyRadix2
{
public:
    /**
     * @brief Calculate FFT of the given function values.
     * @param functionValues Function values, real numbers. Number of values must be power
     * of 2.
     * @return Array of complex numbers - FFT of the input values.
     */
    static std::vector<std::complex<float>> getFFT(const std::vector<float>& functionValues);

    /**
     * @brief Calculate the FFT of the given function values and return magnitudes of the
     * frequencies.
     * @param functionValues Function values, real numbers. Number of values must be power
     * of 2.
     * @return Array of real numbers - magnitude values of frequencies.
     */
    static std::vector<float> getMagnitudes(const std::vector<float>& functionValues);

    /**
     * @brief Calculate magnitude of every frequency for the given FFT.
     * @param fft Input FFT values.
     * @return Array of real numbers - magnitude values of frequencies.
     */
    template<typename T>
    static std::vector<T> getMagnitudesFromFFT(const std::vector<std::complex<T>>& fft)
    {
        std::vector<T> magnitudes;

        for (const auto& complexNumber : fft)
        {
            const auto magnitude =
              2 *
              (std::sqrt(std::pow(complexNumber.real(), 2) + std::pow(complexNumber.imag(), 2)));
            magnitudes.push_back(static_cast<T>(magnitude));
        }

        return magnitudes;
    }
};
}
