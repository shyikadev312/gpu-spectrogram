__kernel void convertToDBFS(
	__global float* magnitudes,
	float referenceValue
	)
{
	uint globalId = get_global_id(0);
	float magnitude = magnitudes[globalId];
	float dbfs = 20.0f * log10(magnitude / referenceValue);
    if (dbfs < -10000.0f)
    {
        dbfs = -10000.0f;
    }
    magnitudes[globalId] = dbfs;
}

__kernel void updateDensityHeatmap(
   __global float2* heatmap,
   __global float* dbfsHistoryBuffer,
   uint heatmapWidth,
   uint heatmapHeight,
   uint historyBufferCount,
   uint mostRecentBufferIndex,
   float magnitudeIndexToDbfsCoeff
   )
{
    const uint frequencyIndex = get_global_id(0);
    const uint magnitudeCellIndex = get_global_id(1);
    
    const float currentCellCorrespondingDBFS = -((float)magnitudeCellIndex) * magnitudeIndexToDbfsCoeff;

    float cellValue = 0;
    
    float dbfsAge = 0;
    float dbfsIntensity = 0;

    for (size_t historyBufferIndex = 0; historyBufferIndex < historyBufferCount; ++historyBufferIndex)
    {
        const size_t historyBufferElementIndex = historyBufferIndex * heatmapWidth + frequencyIndex;
        const float dbfsValue = dbfsHistoryBuffer[historyBufferElementIndex];

        bool isPresent = currentCellCorrespondingDBFS < dbfsValue;
        if (isPresent)
        {
            cellValue += 1.0f;
            dbfsIntensity += 1.0f;

            float bufferAge = 0;
            if (mostRecentBufferIndex >= historyBufferIndex)
            {
                bufferAge = mostRecentBufferIndex - historyBufferIndex;
            }
            else
            {
                bufferAge = historyBufferCount - (historyBufferIndex - mostRecentBufferIndex);
            }

            dbfsAge += bufferAge;
        }
    }
    
    uint heatmapCellIndex = frequencyIndex * heatmapHeight + magnitudeCellIndex;
    heatmap[heatmapCellIndex].x = dbfsIntensity / (float)historyBufferCount;
    heatmap[heatmapCellIndex].y = (dbfsAge / (float)historyBufferCount) / (float)historyBufferCount;
}
