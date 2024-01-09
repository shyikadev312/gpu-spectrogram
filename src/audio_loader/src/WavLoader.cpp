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

BitDepth getBitDepth(size_t bitsPerSample)
{
    switch (bitsPerSample)
    {
        case 16: return BitDepth::Bit16;
        case 32: return BitDepth::Bit32;
        case 64: return BitDepth::Bit64;
        default:
            throw utils::Exception("Unsupported bit depth per sample value: {}", bitsPerSample);
    }
}

} // namespace

AudioData WavLoader::load(std::istream& reader)
{
    WavHeader wavHeader;
    reader.read(reinterpret_cast<char*>(&wavHeader), sizeof(WavHeader));
    // reader.read(wavHeader.signatureRiff, sizeof(wavHeader.signatureRiff));
    // reader >> wavHeader.signatureRiff >> wavHeader.fileSize >> wavHeader.signatureWave;

    if (strncmp(wavHeader.signatureRiff, "RIFF", sizeof(wavHeader.signatureRiff)) != 0)
    {
        throw std::runtime_error("Invalid format Riff signature.");
    }

    // todo check for file size

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

    std::vector<uint16_t> channelsData;
    channelsData.resize(samplesCount * fmtChunk.numberOfChannels);

    if (sampleSize == 1)
    {
        char buffer;
        for (size_t valueIndex = 0; valueIndex < dataChunk.chunkSize; ++valueIndex)
        {
            reader.read(&buffer, sampleSize);
            const int16_t value = static_cast<int16_t>(buffer);
            channelsData[valueIndex] = value;
        }
    }
    else if (sampleSize == 2)
    {
        reader.read(reinterpret_cast<char*>(channelsData.data()), dataChunk.chunkSize);
    }
    else if (sampleSize == 3)
    {
        size_t valueIndex = 0;
        constexpr int32_t max24bit = (256 * 256 * 256) - 1;

        for (size_t i = 0; i < dataChunk.chunkSize; i += 3)
        {
            uint32_t value = 0;
            reader.read(reinterpret_cast<char*>(&value), 3);
            const auto ratio = static_cast<float>(value) / static_cast<float>(max24bit);
            uint16_t finalVal = static_cast<uint16_t>(ratio * std::numeric_limits<uint16_t>::max());
            channelsData[valueIndex] = finalVal;
            ++valueIndex;
        }
    }
    else
    {
        throw std::runtime_error("Unsupported sample size.");
    }

    return AudioData(getBitDepth(fmtChunk.bitsPerSample), fmtChunk.sampleRate, { channelsData });
}
}
