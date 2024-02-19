#include <spectr/audio_loader/WavLoader.h>

#include <spectr/utils/Exception.h>

#include <fmt/format.h>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace spectr::audio_loader
{
namespace
{
constexpr uint16_t WAVE_FORMAT_PCM = 0x0001;
constexpr uint16_t WAVE_FORMAT_EXTENSIBLE = 0xFFFE;

#pragma pack(push, 1)
struct WavHeader
{
    char signatureRiff[4];
    uint32_t fileSize;
    char signatureWave[4];
};

struct FmtChunk
{
    uint16_t audioFormat;
    uint16_t numberOfChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;

    uint16_t getBytesPerSample() { return bitsPerSample / 8; }
};

struct RIFF_Chunk
{
    char chunkName[4];
    int32_t chunkSize;
};

struct WaveFormatExtensibleChunk
{
    uint16_t extensionSize;
    uint16_t validBitsPerSample;
    uint32_t channelMask;
    char subFormatGuid[16];
};

#pragma pack(pop)

SampleDataType getSampleDataType(size_t bitsPerSample)
{
    switch (bitsPerSample)
    {
        case 16: return SampleDataType::Int16;
        case 32: return SampleDataType::Int32;
        case 64: return SampleDataType::Int64;
        default:
            throw utils::Exception("Unsupported bit depth per sample value: {}", bitsPerSample);
    }
}

} // namespace

SignalData WavLoader::load(std::istream& reader)
{
    WavHeader wavHeader;
    reader.read(reinterpret_cast<char*>(&wavHeader), sizeof(WavHeader));

    if (strncmp(wavHeader.signatureRiff, "RIFF", sizeof(wavHeader.signatureRiff)) != 0)
    {
        throw std::runtime_error("Invalid format Riff signature.");
    }

    // TODO check for file size

    if (strncmp(wavHeader.signatureWave, "WAVE", sizeof(wavHeader.signatureWave)) != 0)
    {
        throw std::runtime_error("Invalid format WAVE signature.");
    }

    bool dataChunkFound = false;
    bool fmtChunkFound = false;
    FmtChunk fmtChunk;
    RIFF_Chunk dataChunk;
    RIFF_Chunk chunkHeader;
    WaveFormatExtensibleChunk waveFormatExtensibleChunk;
    memset(&fmtChunk, 0, sizeof(FmtChunk));
    memset(&dataChunk, 0, sizeof(RIFF_Chunk));
    memset(&chunkHeader, 0, sizeof(RIFF_Chunk));

    while (!dataChunkFound)
    {
        reader.read(reinterpret_cast<char*>(&chunkHeader), sizeof(RIFF_Chunk));
        const auto isPaddingByteNeeded = chunkHeader.chunkSize % 2 != 0;

        if (strncmp(chunkHeader.chunkName, "fmt ", sizeof(chunkHeader.chunkName)) == 0)
        {
            reader.read(reinterpret_cast<char*>(&fmtChunk), sizeof(FmtChunk));
            fmtChunkFound = true;

            if (fmtChunk.audioFormat == WAVE_FORMAT_EXTENSIBLE)
            {
                reader.read(reinterpret_cast<char*>(&waveFormatExtensibleChunk),
                            sizeof(WaveFormatExtensibleChunk));

                if (waveFormatExtensibleChunk.extensionSize != 22)
                {
                    throw std::runtime_error("Failed to parse WAVE extensible format.");
                }
            }
            else
            {
                const auto diffLeft = chunkHeader.chunkSize - sizeof(FmtChunk);
                reader.ignore(diffLeft);
            }
        }
        else if (strncmp(chunkHeader.chunkName, "data", sizeof(chunkHeader.chunkName)) == 0)
        {
            dataChunk = chunkHeader;
            dataChunkFound = true;
        }
        else
        {
            auto bytesToSkip = chunkHeader.chunkSize;
            if (isPaddingByteNeeded)
            {
                ++bytesToSkip;
            }

            reader.ignore(bytesToSkip);
        }
    }

    if (dataChunkFound && !fmtChunkFound)
    {
        throw std::runtime_error("fmt chunk is required but not found.");
    }

    if (fmtChunk.audioFormat == WAVE_FORMAT_EXTENSIBLE)
    {
        const uint16_t subFormat =
          *(reinterpret_cast<uint16_t*>(waveFormatExtensibleChunk.subFormatGuid));

        if (subFormat == WAVE_FORMAT_PCM)
        {
            fmtChunk.audioFormat = WAVE_FORMAT_PCM;
        }
    }

    if (fmtChunk.audioFormat != WAVE_FORMAT_PCM)
    {
        throw std::runtime_error(
          fmt::format("Unsupported WAVE format = {}. Only PCM supported.", fmtChunk.audioFormat));
    }

    const auto sampleSize = fmtChunk.getBytesPerSample();
    const auto singleChannelDataSize = dataChunk.chunkSize / fmtChunk.numberOfChannels;
    const auto samplesCount = singleChannelDataSize / sampleSize;

    std::vector<SampleDataVariant> channelsData;
    channelsData.reserve(fmtChunk.numberOfChannels);

    if (sampleSize == 2)
    {
        for (size_t i = 0; i < fmtChunk.numberOfChannels; ++i)
        {
            SampleData16 samples;
            samples.resize(samplesCount);
            reader.read(reinterpret_cast<char*>(samples.data()), samplesCount);
            channelsData.push_back(std::move(samples));
        }
    }
    else
    {
        throw utils::Exception("Unsupported sample size: ", sampleSize);
    }

    return SignalData(fmtChunk.sampleRate, std::move(channelsData));
}
}
