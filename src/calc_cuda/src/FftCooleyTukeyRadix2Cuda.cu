#ifndef BIT_REVERSE_SHIFT_VALUE
# define BIT_REVERSE_SHIFT_VALUE 26
#endif

#include <builtin_types.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <complex>

unsigned int bitReverse(unsigned int v)
{
    v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
    v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
    v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
    v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
    v = (v >> 16) | (v << 16);

    v = v >> BIT_REVERSE_SHIFT_VALUE;

    return v;
}

float2 complexMultiply(float2 a, float2 b) {
    return (float2)(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

__global__ void bit_reverse_permutation(
    const float2* input,
    float2* output
) {
    unsigned int globalId = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int i1 = globalId;
    unsigned int i2 = bitReverse(globalId);
    output[i2] = input[i1];
}

__global__ void fft_stage(
    const float2* input,
    float2* output,
    const float2* omegaValues,
    unsigned int subFftSize,
    unsigned int subFftCount,
    unsigned int stageIndex
) {
    const unsigned int subFftIndex = blockIdx.x * blockDim.x + threadIdx.x;
    const unsigned int subFftElementIndex = blockIdx.y * blockDim.y + threadIdx.y;
    const unsigned int subFftHalfSize = subFftSize / 2;

    const unsigned int index1 = subFftIndex * subFftSize + subFftElementIndex;
    const unsigned int index2 = index1 + subFftHalfSize;

    const float2 input1 = input[index1];
    const float2 input2 = input[index2];

    const float2 omegaK = omegaValues[subFftElementIndex];
    const float2 mul = complexMultiply(omegaK, input2);
    float2 y1 = input1;
    float2 y2 = input1;

    y1.x += mul.x;
    y1.y += mul.y;
    y2.x -= mul.x;
    y2.y -= mul.y;

    output[index1] = y1;
    output[index2] = y2;
}

__global__ void calculate_magnitudes(
    const float2* fft,
    float* magnitudes
) {
    const unsigned int i = blockIdx.x * blockDim.x + threadIdx.x;
    magnitudes[i] = 2 * sqrt((float)(pow(fft[i].x, 2) + pow(fft[i].y, 2)));
}

__global__ void find_max(
    const float* values,
    __shared__ float* temp,
    float* output
) {
    const size_t globalId = blockIdx.x * blockDim.x + threadIdx.x;
    const size_t localId = threadIdx.x;
    const size_t localSize = blockDim.x;

    temp[localId] = values[globalId];

    for (int i = localSize >> 1; i > 0; i >>= 1)
    {
        if (localId < i)
        {
            if (temp[localId] < temp[localId + i])
            {
                temp[localId] = temp[localId + i];
            }
        }
    }

    if (localId == 0)
    {
        output[blockIdx.x] = temp[0];
    }
}
