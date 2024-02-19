#define _USE_MATH_DEFINES

#include <spectr/calc_cpu/FftCooleyTukeyUtils.h>

#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>
#include <vector>

constexpr auto FftSizeTwoPower = 5;

template<typename T>
void print(const std::vector<std::complex<T>>& complexValues)
{
    for (const auto& val : complexValues)
    {
        const auto real = static_cast<float>(val.real());
        const auto imag = static_cast<float>(val.imag());
        std::cout << "{ ";
        const auto precision = std::numeric_limits<float>::digits10 + 1;
        std::cout << std::setprecision(precision) << real << ", " << imag << " },\n";
    }
}

int main()
{
    using Complex = std::complex<double>;

    std::vector<Complex> omegaValues;
    for (size_t i = 0; i < FftSizeTwoPower + 1; ++i)
    {
        const auto omega = spectr::calc_cpu::FftCooleyTukeyUtils::getOmegaMultiplier<double>(i - 1);
        omegaValues.push_back(omega);
    }

    std::cout << "Omega values:\n";
    print(omegaValues);
    std::cout << "\n\n";

    for (size_t stageIndex = 0; stageIndex < FftSizeTwoPower; ++stageIndex)
    {
        std::cout << "// Stage " << (stageIndex + 1) << " omega values:\n";

        const auto omegas = spectr::calc_cpu::FftCooleyTukeyUtils::getOmegas<double>(stageIndex);
        print(omegas);
    }

    return EXIT_SUCCESS;
}
