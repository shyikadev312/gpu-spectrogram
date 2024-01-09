#define _USE_MATH_DEFINES
#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>
#include <vector>

constexpr auto FftSizeTwoPower = 18;

using Complex = std::complex<double>;

void print(const std::vector<Complex>& values)
{
    for (const auto& val : values)
    {
        const auto real = static_cast<float>(val.real());
        const auto imag = static_cast<float>(val.imag());
        std::cout << "{ ";
        const auto prec = std::numeric_limits<float>::digits10 + 1;
        // std::cout << std::setw(prec + 3);
        std::cout << std::setprecision(prec) << real << ", " << imag << " },\n";
    }
}

int main()
{
    std::vector<Complex> omegaValues;
    for (size_t i = 1; i < FftSizeTwoPower + 1; ++i)
    {
        const auto real = std::cos(2.0 * M_PI / std::pow(2.0, i));
        const auto imag = -std::sin(2.0 * M_PI / std::pow(2.0, i));
        const Complex val{ real, imag };
        omegaValues.push_back(val);
    }

    std::cout << "Omega values:\n";
    print(omegaValues);
    std::cout << "\n\n";

    for (size_t stageIndex = 0; stageIndex < FftSizeTwoPower; ++stageIndex)
    {
        std::cout << "// Stage " << (stageIndex + 1) << " omega values:\n";

        Complex omegaK{ 1 };
        std::vector<Complex> wns;
        wns.push_back(omegaK);

        const auto omegaMultiplier = omegaValues[stageIndex];

        for (size_t subFftElementIndex = 0; subFftElementIndex < stageIndex; ++subFftElementIndex)
        {
            omegaK *= omegaMultiplier;
            wns.push_back(omegaK);
        }

        while (wns.size() < FftSizeTwoPower)
        {
            wns.push_back(0);
        }

        print(wns);
    }

    return EXIT_SUCCESS;
}
