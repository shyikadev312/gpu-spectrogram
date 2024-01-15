#include <spectr/calc_cpu/FftCooleyTukeyRadix2.h>

#include <gtest/gtest.h>

namespace spectr::calc_cpu::test
{
namespace
{
using Complex = std::complex<float>;
constexpr float Eps = 1e-5f;

void ExpectNear(const Complex& c1, const Complex& c2)
{
    EXPECT_NEAR(c1.real(), c2.real(), Eps);
    EXPECT_NEAR(c1.imag(), c2.imag(), Eps);
}
}

TEST(FftCooleyTukeyRadix2Test, TwoNaturalNumbers)
{
    const auto v = FftCooleyTukeyRadix2::getFFT({ 1, 2 });
    ExpectNear(v[0], Complex{ 3 });
    ExpectNear(v[1], Complex{ -1 });
}

TEST(FftCooleyTukeyRadix2Test, FourNaturalNumbers)
{
    const auto v = FftCooleyTukeyRadix2::getFFT({ 1, 2, 3, 4 });
    ExpectNear(v[0], Complex{ 10 });
    ExpectNear(v[1], Complex{ -2, 2 });
    ExpectNear(v[2], Complex{ -2 });
    ExpectNear(v[3], Complex{ -2, -2 });
}

TEST(FftCooleyTukeyRadix2Test, EightNaturalNumbers)
{
    const auto v = FftCooleyTukeyRadix2::getFFT({ 1, 2, 3, 4, 5, 6, 7, 8 });
    ExpectNear(v[0], Complex{ 36 });
    ExpectNear(v[1], Complex{ -4, 9.656854f });
    ExpectNear(v[2], Complex{ -4, 4 });
    ExpectNear(v[3], Complex{ -4, 1.656854f });
    ExpectNear(v[4], Complex{ -4, 0 });
    ExpectNear(v[5], Complex{ -4, -1.656854f });
    ExpectNear(v[6], Complex{ -4, -4 });
    ExpectNear(v[7], Complex{ -4, -9.656854f });
}
}
