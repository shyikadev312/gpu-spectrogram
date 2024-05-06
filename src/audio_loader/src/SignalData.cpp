#include <spectr/audio_loader/SignalData.h>

#include <spectr/utils/Assert.h>

#include <stdexcept>

namespace spectr::audio_loader
{
namespace
{
template<typename TSampleData>
size_t getSampleCountT(const TSampleData& sampleData)
{
    // ASSERT(!sampleData.empty());
    return sampleData.size();
}

size_t getSampleCount(const SampleDataVariant& sampleData)
{
    size_t count = 0;
    std::visit(
      [&](auto&& arg)
      {
          using T = std::decay_t<decltype(arg)>;
          count = getSampleCountT<T>(arg);
      },
      sampleData);

    return count;
}

SampleDataType getSampleDataType(const SampleDataVariant& data)
{
    if (std::holds_alternative<SampleData16>(data))
    {
        return SampleDataType::Int16;
    }
    if (std::holds_alternative<SampleData32>(data))
    {
        return SampleDataType::Int32;
    }
    if (std::holds_alternative<SampleData64>(data))
    {
        return SampleDataType::Int64;
    }
    if (std::holds_alternative<SampleDataFloat>(data))
    {
        return SampleDataType::Float;
    }
    throw std::runtime_error("Sample data type not recognized.");
}
}

SignalData::SignalData(size_t sampleRate, std::vector<SampleDataVariant> channelsDatas)
  : m_channelsDatas{ std::move(channelsDatas) }
  , m_sampleRate{ sampleRate }
  , m_sampleCount{ audio_loader::getSampleCount(m_channelsDatas[0]) }
{
    ASSERT(!m_channelsDatas.empty());

    const auto& referenceData = m_channelsDatas[0];
    for (const auto& channelData : m_channelsDatas)
    {
        if (audio_loader::getSampleCount(referenceData) !=
            audio_loader::getSampleCount(channelData))
        {
            throw std::runtime_error("All data channels must be the same size.");
        }

        if (audio_loader::getSampleDataType(referenceData) !=
            audio_loader::getSampleDataType(channelData))
        {
            throw std::runtime_error("All data channels must have the same data type of sample.");
        }
    }
}

size_t SignalData::getSampleRate() const
{
    return m_sampleRate;
}

size_t SignalData::getSampleCount() const
{
    return m_sampleCount;
}

SampleDataType SignalData::getSampleDataType() const
{
    return audio_loader::getSampleDataType(m_channelsDatas[0]);
}

size_t SignalData::getChannelCount() const
{
    return m_channelsDatas.size();
}

const SampleDataVariant& SignalData::getChannelSampleData(size_t channelIndex) const
{
    return m_channelsDatas[channelIndex];
}

const SampleData16& SignalData::getSampleData16(size_t channelIndex) const
{
    return std::get<SampleData16>(m_channelsDatas[channelIndex]);
}

const SampleData32& SignalData::getSampleData32(size_t channelIndex) const
{
    return std::get<SampleData32>(m_channelsDatas[channelIndex]);
}

const SampleData64& SignalData::getSampleData64(size_t channelIndex) const
{
    return std::get<SampleData64>(m_channelsDatas[channelIndex]);
}

const SampleDataFloat& SignalData::getSampleDataFloat(size_t channelIndex) const
{
    return std::get<SampleDataFloat>(m_channelsDatas[channelIndex]);
}

float SignalData::getDuration() const
{
    return static_cast<float>(m_sampleCount) / m_sampleRate;
}

SignalData& SignalData::operator+=(SignalData data) {
    if (data.m_channelsDatas.size() == 0) {
        return *this;
    }

    if (data.m_channelsDatas.size() < m_channelsDatas.size()) {
        throw std::runtime_error("Data must have the same or a higher amount of channels");
    }

    // Copy sample rate from other sample if this has no data yet
    if (m_channelsDatas.size() == 0) {
        m_sampleRate = data.m_sampleRate;
    }

    for (size_t i = m_channelsDatas.size(); i < data.m_channelsDatas.size(); i++) {
        auto try_creating = [&]<typename SampleType>() {
            if (std::holds_alternative<SampleType>(data.m_channelsDatas[i])) {
                m_channelsDatas.push_back(SampleType { });
            }
        };

        try_creating.operator()<SampleData16>();
        try_creating.operator()<SampleData32>();
        try_creating.operator()<SampleData64>();
        try_creating.operator()<SampleDataFloat>();
    }

    for (size_t i = 0; i < m_channelsDatas.size(); i++) {
        auto& channel1 = m_channelsDatas[i];
        auto& channel2 = data.m_channelsDatas[i];

        auto try_adding = [&]<typename SampleType>() {
            if (std::holds_alternative<SampleType>(channel1)) {
                if (!std::holds_alternative<SampleType>(channel2)) {
                    throw std::runtime_error("Data must have the same type");
                }

                auto& src = std::get<SampleType>(channel2);
                auto& dst = std::get<SampleType>(channel1);

                dst.insert(dst.end(), src.begin(), src.end());
            }
        };

        try_adding.operator()<SampleData16>();
        try_adding.operator()<SampleData32>();
        try_adding.operator()<SampleData64>();
        try_adding.operator()<SampleDataFloat>();
    }

    m_sampleCount += data.getSampleCount();

    return *this;
}
}
