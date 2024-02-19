#pragma once

#include <complex>
#include <vector>

#include <spectr/utils/Math.h>

namespace spectr::calc_cpu
{
class FftCooleyTukeyUtils
{
public:
    template<typename T>
    static std::complex<T> getOmegaMultiplier(size_t stageIndex)
    {
        const auto real = std::cos(2.0 * utils::Math::PI / std::pow(2.0, stageIndex + 1));
        const auto imag = -std::sin(2.0 * utils::Math::PI / std::pow(2.0, stageIndex + 1));
        return { static_cast<T>(real), static_cast<T>(imag) };
    };

    /**
     * @brief Create FFT coefficients for stage N.
     * @details (Twiddle factor) (roots of unity) (ω). https://en.wikipedia.org/wiki/Twiddle_factor
     */
    template<typename T>
    static std::vector<std::complex<T>> getOmegas(size_t stageIndex)
    {
        const auto omegaCount = 1 << stageIndex;

        std::vector<std::complex<T>> omegas;
        omegas.reserve(omegaCount);

        std::complex<T> omegaK{ 1 };
        omegas.push_back(omegaK);

        const auto omegaMultiplier = getOmegaMultiplier<T>(stageIndex);
        for (size_t k = 1; k < omegaCount; ++k)
        {
            omegaK *= omegaMultiplier;
            omegas.push_back(omegaK);
        }

        return omegas;
    }
};
}
