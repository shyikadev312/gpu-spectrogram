#include <spectr/calc_opencl/FftCooleyTukeyRadix2.h>

#include <spectr/calc_opencl/OpenclManager.h>
#include <spectr/calc_opencl/OpenclUtils.h>

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <vector>

namespace spectr::calc_opencl::test
{
namespace
{
const double Eps = 1e-5;

class FftCooleyTukeyRadix2Test : public ::testing::Test
{
public:
    std::vector<float> executeTest(const std::vector<float>& inputRealValues)
    {
        spdlog::set_level(spdlog::level::debug);

        OpenclManager openclManager;
        openclManager.initContext();
        auto context = openclManager.getContext();

        // OpenclUtils::printContextInfo(context, std::cout);

        FftCooleyTukeyRadix2 fftOpenCl(context, inputRealValues.size());
        fftOpenCl.execute(inputRealValues);
        const auto v = fftOpenCl.getFinalDataBufferCpu();
        return v;
    }
};
}

TEST_F(FftCooleyTukeyRadix2Test, TwoNaturalNumbers)
{
    const auto v = executeTest({ 1, 2 });
    EXPECT_NEAR(v[0], 3, Eps);
    EXPECT_NEAR(v[1], -1, Eps);
}

TEST_F(FftCooleyTukeyRadix2Test, FourNaturalNumbers)
{
    const auto v = executeTest({ 1, 2, 3, 4 });
    EXPECT_NEAR(v[0], 10, Eps);
    EXPECT_NEAR(v[1], -2, Eps);
    EXPECT_NEAR(v[2], -2, Eps);
    EXPECT_NEAR(v[3], -2, Eps);
}

TEST_F(FftCooleyTukeyRadix2Test, EightNaturalNumbers)
{
    const auto v = executeTest({ 1, 2, 3, 4, 5, 6, 7, 8 });
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
