#pragma once

#define _USE_MATH_DEFINES

#include <cmath>
#include <complex>
#include <vector>

namespace spectr::calc_cpu
{
template<typename T>
class FftCooleyTukeyUtils
{
public:
    static std::complex<T> getOmegaMultiplier(size_t stageIndex)
    {
        const auto real = std::cos(2.0 * M_PI / std::pow(2.0, stageIndex + 1));
        const auto imag = -std::sin(2.0 * M_PI / std::pow(2.0, stageIndex + 1));
        return { static_cast<T>(real), static_cast<T>(imag) };
    };

    static std::vector<std::complex<T>> getOmegas(size_t stageIndex)
    {
        std::vector<std::complex<T>> omegas;

        const auto omegaCount = 1 << stageIndex;
        std::complex<T> omegaK{ 1 };
        omegas.push_back(omegaK);

        const auto omegaMultiplier = getOmegaMultiplier(stageIndex);

        for (size_t k = 1; k < omegaCount; ++k)
        {
            omegaK *= omegaMultiplier;
            omegas.push_back(omegaK);
        }

        return omegas;
    }
};
}
