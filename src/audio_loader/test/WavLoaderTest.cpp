#include <spectr/audio_loader/WavLoader.h>

#include <spectr/utils/Asset.h>

#include <gtest/gtest.h>

#include <fstream>

namespace spectr::audio_loader::test
{
TEST(WavLoader, Sample1)
{
    std::ifstream file(utils::Asset::getPath("samples/440Hz_44100Hz_16bit_05sec.wav"));
    const auto audio = WavLoader::load(file);
    EXPECT_EQ(audio.getSampleDataType(), SampleDataType::Int16);
    EXPECT_EQ(audio.getSampleRate(), 44100);
    EXPECT_EQ(audio.getChannelCount(), 1);
    EXPECT_EQ(audio.getSampleCount(), 44100 * 5);
    EXPECT_NEAR(audio.getDuration(), 5.0f, 0.1f);

    EXPECT_NO_THROW(audio.getSampleData16(0));
    EXPECT_THROW(audio.getSampleData32(0), std::bad_variant_access);
    EXPECT_THROW(audio.getSampleData64(0), std::bad_variant_access);
}
}
