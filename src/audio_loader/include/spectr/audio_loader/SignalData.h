#pragma once

#include <cstdint>
#include <cstdlib>
#include <variant>
#include <vector>

namespace spectr::audio_loader
{
template<typename T>
using SampleData = std::vector<T>;

using SampleData16 = SampleData<int16_t>;
using SampleData32 = SampleData<int32_t>;
using SampleData64 = SampleData<int64_t>;
using SampleDataFloat = SampleData<float>;
using SampleDataVariant = std::variant<SampleData16, SampleData32, SampleData64, SampleDataFloat>;

enum class SampleDataType
{
    DataNone,
    Int16,
    Int32,
    Int64,
    Float,
    Double,
};

/**
 * @brief Container of raw signal data for N channels.
 */
class SignalData
{
public:
    SignalData() = default;

    SignalData(size_t sampleRate, std::vector<SampleDataVariant> channelsDatas);

    size_t getSampleRate() const;

    size_t getSampleCount() const;

    /**
     * @brief Get data type which is used for storing the signal values.
     */
    SampleDataType getSampleDataType() const;

    size_t getChannelCount() const;

    const SampleDataVariant& getChannelSampleData(size_t channelIndex) const;

    const SampleData16& getSampleData16(size_t channelIndex) const;

    const SampleData32& getSampleData32(size_t channelIndex) const;

    const SampleData64& getSampleData64(size_t channelIndex) const;

    const SampleDataFloat& getSampleDataFloat(size_t channelIndex) const;

    /**
     * @brief Returns approximate duration of the stored audio data in seconds.
     */
    float getDuration() const;

    SignalData& operator+=(SignalData data);

private:
    std::vector<SampleDataVariant> m_channelsDatas;
    size_t m_sampleRate = 0;
    size_t m_sampleCount = 0;
};
}
