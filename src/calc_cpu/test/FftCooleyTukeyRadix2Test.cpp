#include <spectr/calc_cpu/FftCooleyTukeyRadix2.h>

#include <gtest/gtest.h>

namespace spectr::calc_cpu::test
{
namespace
{
const double Eps = 1e-5;
}

TEST(FftCooleyTukeyRadix2Test, TwoNaturalNumbers)
{
    const auto v = FftCooleyTukeyRadix2::execute({ 1, 2 });
    EXPECT_NEAR(v[0], 3, Eps);
    EXPECT_NEAR(v[1], -1, Eps);
}

TEST(FftCooleyTukeyRadix2Test, FourNaturalNumbers)
{
    const auto v = FftCooleyTukeyRadix2::execute({ 1, 2, 3, 4 });
    EXPECT_NEAR(v[0], 10, Eps);
    EXPECT_NEAR(v[1], -2, Eps);
    EXPECT_NEAR(v[2], -2, Eps);
    EXPECT_NEAR(v[3], -2, Eps);
}

TEST(FftCooleyTukeyRadix2Test, EightNaturalNumbers)
{
    const auto v = FftCooleyTukeyRadix2::execute({ 1, 2, 3, 4, 5, 6, 7, 8 });
    EXPECT_NEAR(v[0], 36, Eps);
    EXPECT_NEAR(v[1], -4, Eps);
    EXPECT_NEAR(v[2], -4, Eps);
    EXPECT_NEAR(v[3], -4, Eps);
    EXPECT_NEAR(v[4], -4, Eps);
    EXPECT_NEAR(v[5], -4, Eps);
    EXPECT_NEAR(v[6], -4, Eps);
    EXPECT_NEAR(v[7], -4, Eps);
}
}
