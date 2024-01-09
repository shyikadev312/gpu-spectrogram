#include <spectr/audio_loader/AudioData.h>

#include <spectr/utils/Assert.h>

namespace spectr::audio_loader
{
namespace
{
template<typename TSampleData>
size_t getSampleCountT(const TSampleData& sampleData)
{
    ASSERT(!sampleData.empty());
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
}

AudioData::AudioData(BitDepth bitDepth,
                     size_t sampleRate,
                     std::vector<SampleDataVariant> channelsDatas)
  : m_bitDepth{ bitDepth }
  , m_sampleRate{ sampleRate }
  , m_sampleCount{ audio_loader::getSampleCount(channelsDatas[0]) }
  , m_channelsDatas{ std::move(channelsDatas) }
{
    ASSERT(!m_channelsDatas.empty());
    const auto channelSampleCount = audio_loader::getSampleCount(m_channelsDatas[0]);
    for (const auto& channelData : m_channelsDatas)
    {
        ASSERT(channelSampleCount == audio_loader::getSampleCount(channelData));
    }
}

size_t AudioData::getSampleRate() const
{
    return m_sampleRate;
}

size_t AudioData::getSampleCount() const
{
    return m_sampleCount;
}

BitDepth AudioData::getBitDepth() const
{
    return m_bitDepth;
}

size_t AudioData::getChannelCount() const
{
    return m_channelsDatas.size();
}

const SampleDataVariant& AudioData::getChannelSampleData(size_t channelIndex) const
{
    return m_channelsDatas[channelIndex];
}

const SampleData16& AudioData::getSampleData16(size_t channelIndex) const
{
    return std::get<SampleData16>(m_channelsDatas[channelIndex]);
}

const SampleData32& AudioData::getSampleData32(size_t channelIndex) const
{
    return std::get<SampleData32>(m_channelsDatas[channelIndex]);
}

const SampleData64& AudioData::getSampleData64(size_t channelIndex) const
{
    return std::get<SampleData64>(m_channelsDatas[channelIndex]);
}

float AudioData::getDuration() const
{
    return static_cast<float>(m_sampleCount) / m_sampleRate;
}
}
