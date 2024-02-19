#include <spectr/calc_opencl/FftCooleyTukeyRadix2CL.h>

#include <spectr/calc_opencl/OpenclManager.h>
#include <spectr/calc_opencl/OpenclUtils.h>

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <vector>

namespace spectr::calc_opencl::test
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

class FftCooleyTukeyRadix2Test : public ::testing::Test
{
public:
    std::vector<Complex> executeTest(const std::vector<float>& inputRealValues)
    {
        spdlog::set_level(spdlog::level::debug);

        OpenclManager openclManager;
        auto context = openclManager.getContext();

        // OpenclUtils::printContextInfo(context, std::cout);

        FftCooleyTukeyRadix2 fftOpenCl(context, inputRealValues.size());
        fftOpenCl.execute(inputRealValues);
        const auto v = fftOpenCl.getFffBufferCpu();
        return v;
    }
};
}

TEST_F(FftCooleyTukeyRadix2Test, TwoNaturalNumbers)
{
    const auto v = executeTest({ 1, 2 });
    ExpectNear(v[0], Complex{ 3 });
    ExpectNear(v[1], Complex{ -1 });
}

TEST_F(FftCooleyTukeyRadix2Test, FourNaturalNumbers)
{
    const auto v = executeTest({ 1, 2, 3, 4 });
    ExpectNear(v[0], Complex{ 10 });
    ExpectNear(v[1], Complex{ -2, 2 });
    ExpectNear(v[2], Complex{ -2 });
    ExpectNear(v[3], Complex{ -2, -2 });
}

TEST_F(FftCooleyTukeyRadix2Test, EightNaturalNumbers)
{
    const auto v = executeTest({ 1, 2, 3, 4, 5, 6, 7, 8 });
    ExpectNear(v[0], Complex{ 36 });
    ExpectNear(v[1], Complex{ -4, 9.656854f });
    ExpectNear(v[1], Complex{ -4, 4 });
    ExpectNear(v[1], Complex{ -4, 1.656854f });
    ExpectNear(v[1], Complex{ -4, 0 });
    ExpectNear(v[1], Complex{ -4, -1.656854f });
    ExpectNear(v[1], Complex{ -4, -4 });
    ExpectNear(v[1], Complex{ -4, -9.656854f });
}
}
