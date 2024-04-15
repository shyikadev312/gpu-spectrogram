#include <builtin_types.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <complex>
__global__ void convertToDBFS(
    float* magnitudes,
    float referenceValue
) {
    unsigned int globalId = blockIdx.x * blockDim.x + threadIdx.x;
    float magnitude = magnitudes[globalId];
    float dbfs = 20.0f * log10(magnitude / referenceValue);
    if (dbfs < -10000.0f)
    {
        dbfs = -10000.0f;
    }
    magnitudes[globalId] = dbfs;
}

__global__ void updateDensityHeatmap(
    float2* heatmap,
    float* dbfsHistoryBuffer,
    unsigned int heatmapWidth,
    unsigned int heatmapHeight,
    unsigned int historyBufferCount,
    unsigned int mostRecentBufferIndex,
    float magnitudeIndexToDbfsCoeff
) {
    const unsigned int frequencyIndex = blockIdx.x * blockDim.x + threadIdx.x;
    const unsigned int magnitudeCellIndex = blockIdx.y * blockDim.y + threadIdx.y;

    const float currentCellCorrespondingDBFS = -((float)magnitudeCellIndex) * magnitudeIndexToDbfsCoeff;

    float cellValue = 0;

    float dbfsAge = 0;
    float dbfsIntensity = 0;

    for (size_t historyBufferIndex = 0; historyBufferIndex < historyBufferCount; ++historyBufferIndex)
    {
        const size_t historyBufferElementIndex = historyBufferIndex * heatmapWidth + frequencyIndex;
        const float dbfsValue = dbfsHistoryBuffer[historyBufferElementIndex];

        bool isPresent = (currentCellCorrespondingDBFS * 0.95f > dbfsValue) && (currentCellCorrespondingDBFS * 1.05f < dbfsValue);
        if (isPresent)
        {
            cellValue += 1.0f;
            dbfsIntensity += 1.0f;

            float bufferAge = 0;
            if (mostRecentBufferIndex >= historyBufferIndex)
            {
                bufferAge = mostRecentBufferIndex - historyBufferIndex;
            } else
            {
                bufferAge = historyBufferCount - (historyBufferIndex - mostRecentBufferIndex);
            }

            dbfsAge += bufferAge;
        }
    }

    unsigned int heatmapCellIndex = frequencyIndex * heatmapHeight + magnitudeCellIndex;
    heatmap[heatmapCellIndex].x = dbfsIntensity / (float)historyBufferCount;
    heatmap[heatmapCellIndex].y = (dbfsAge / (float)historyBufferCount) / (float)historyBufferCount;
}
