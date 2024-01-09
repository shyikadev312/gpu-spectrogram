#pragma once

#include <cstdint>
#include <cstdlib>
#include <variant>
#include <vector>

namespace spectr::audio_loader
{
template<typename T>
using SampleData = std::vector<T>;

using SampleData16 = SampleData<uint16_t>;
using SampleData32 = SampleData<uint32_t>;
using SampleData64 = SampleData<uint64_t>;
using SampleDataVariant = std::variant<SampleData16, SampleData32, SampleData64>;

enum class BitDepth
{
    None,
    Bit16,
    Bit32,
    Bit64,
};

class AudioData
{
public:
    AudioData() = default;

    AudioData(BitDepth bitDepth, size_t sampleRate, std::vector<SampleDataVariant> channelsDatas);

    size_t getSampleRate() const;

    size_t getSampleCount() const;

    BitDepth getBitDepth() const;

    size_t getChannelCount() const;

    const SampleDataVariant& getChannelSampleData(size_t channelIndex) const;

    const SampleData16& getSampleData16(size_t channelIndex) const;

    const SampleData32& getSampleData32(size_t channelIndex) const;

    const SampleData64& getSampleData64(size_t channelIndex) const;

    /**
     * @brief Returns approximate duration of the stored audio data in seconds.
     */
    float getDuration() const;

private:
    BitDepth m_bitDepth = BitDepth::None;
    size_t m_sampleRate = 0;
    size_t m_sampleCount = 0;
    std::vector<SampleDataVariant> m_channelsDatas;
};

}
