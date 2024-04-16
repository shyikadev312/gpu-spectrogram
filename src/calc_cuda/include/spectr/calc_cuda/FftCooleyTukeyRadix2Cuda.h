#pragma once

extern void bit_reverse_permutation_wrapper(const float* input,
                                                  float* output,
                                                  size_t  size);

extern void fft_stage_wrapper(const float* input,
                                    float* output,
                              const float* omegaValues,
                              unsigned int subFftSize,
                              unsigned int subFftCount,
                              unsigned int stageIndex,
                              size_t input_size,
                              size_t omega_size);

extern void convert_to_dbfs_wrapper(float* magnitudes,
                                  float referenceValue,
                                  size_t size);

extern void update_density_heatmap_wrapper(float* heatmap,
                                         float* dbfsHistoryBuffer,
                                         unsigned int heatmapWidth,
                                         unsigned int heatmapHeight,
                                         unsigned int historyBufferCount,
                                         unsigned int mostRecentBufferIndex,
                                         float magnitudeIndexToDbfsCoeff,
                                         size_t size,
                                         size_t historyBufferSize);

extern void calculate_magnitudes_wrapper(const float* fft,
                                               float* magnitudes,
                                               size_t size);

extern void find_max_wrapper(const float* values,
                                   float* output,
                                   size_t size);
