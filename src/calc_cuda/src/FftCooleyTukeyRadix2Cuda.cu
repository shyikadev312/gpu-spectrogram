#ifndef BIT_REVERSE_SHIFT_VALUE
# define BIT_REVERSE_SHIFT_VALUE 26
#endif

#include <builtin_types.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <complex>

__global__ void bit_reverse_permutation(
    const float2* input,
          float2* output
) {
    unsigned int globalId = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned int i1 = globalId;
    unsigned int i2 = globalId;

    i2 = ((i2 >> 1) & 0x55555555) | ((i2 & 0x55555555) << 1);
    i2 = ((i2 >> 2) & 0x33333333) | ((i2 & 0x33333333) << 2);
    i2 = ((i2 >> 4) & 0x0F0F0F0F) | ((i2 & 0x0F0F0F0F) << 4);
    i2 = ((i2 >> 8) & 0x00FF00FF) | ((i2 & 0x00FF00FF) << 8);
    i2 = (i2 >> 16) | (i2 << 16);

    i2 = i2 >> BIT_REVERSE_SHIFT_VALUE;

    output[i2] = input[i1];
}

void bit_reverse_permutation_wrapper(const float* input,
                                           float* output,
                                           size_t size,
                                           size_t block_size) {
    float2* in, *out;

    cudaMalloc((void**)&in,  sizeof(float2) * size);
    cudaMalloc((void**)&out, sizeof(float2) * size);

    cudaMemcpy(in, input, sizeof(float2) * size, cudaMemcpyHostToDevice);

    bit_reverse_permutation<<<block_size, size / block_size>>>(in, out);

    cudaDeviceSynchronize();

    cudaMemcpy(output, out, sizeof(float2) * size, cudaMemcpyDeviceToHost);

    cudaFree(in);
    cudaFree(out);
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
    float2 mul;

    mul.x = omegaK.x * input2.x - omegaK.y * input2.y;
    mul.y = omegaK.x * input2.y + omegaK.y * input2.x;

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

void calculate_magnitudes_wrapper(const float* fft,
                                        float* magnitudes,
                                        size_t size,
                                        size_t block_size) {
    float2* in;
    float* out;

    cudaMalloc((void**)&in,  sizeof(float2) * size);
    cudaMalloc((void**)&out, sizeof(float ) * size);

    cudaMemcpy(in, fft, sizeof(float2) * size, cudaMemcpyHostToDevice);

    calculate_magnitudes<<<block_size, size / block_size>>>(in, out);

    cudaDeviceSynchronize();

    cudaMemcpy(magnitudes, out, sizeof(float) * size, cudaMemcpyDeviceToHost);

    cudaFree(in);
    cudaFree(out);
}


__global__ void find_max(
    const float* values,
    float* temp,
    float* output
) {
    const size_t globalId = blockIdx.x * blockDim.x + threadIdx.x;
    const size_t localId = threadIdx.x;
    const size_t localSize = blockDim.x;

    temp[localId] = values[globalId];
    // barrier(CLK_LOCAL_MEM_FENCE);

    for (int i = localSize >> 1; i > 0; i >>= 1)
    {
        if (localId < i)
        {
            if (temp[localId] < temp[localId + i])
            {
                temp[localId] = temp[localId + i];
            }

            // barrier(CLK_LOCAL_MEM_FENCE);
        }
    }

    if (localId == 0)
    {
        output[blockIdx.x] = temp[0];
    }
}

void find_max_wrapper(const float* values,
                            float* output,
                            size_t size,
                            size_t block_size) {
    float* in, *tmp, *out;

    cudaMalloc((void**)&in,  sizeof(float) * size);
    cudaMalloc((void**)&tmp, sizeof(float) * size);
    cudaMalloc((void**)&out, sizeof(float) * size);

    cudaMemcpy(in, values, sizeof(float) * size, cudaMemcpyHostToDevice);

    find_max<<<block_size, size / block_size>>>(in, tmp, out);

    cudaDeviceSynchronize();

    cudaMemcpy(output, out, sizeof(float) * size, cudaMemcpyDeviceToHost);

    cudaFree(in);
    cudaFree(tmp);
    cudaFree(out);
}

// todo: don't copy each time.
void fft_stage_wrapper(const float* input,
                             float* output,
                       const float* omegaValues,
                       unsigned int subFftSize,
                       unsigned int subFftCount,
                       unsigned int stageIndex,
                       size_t input_size,
                       size_t omega_size) {
    float2* in, *out, *omega;

    dim3 blocks(subFftCount, subFftSize / 2, 1);
    dim3 threads(1, 1, 1);

    cudaMalloc((void**)&in,    sizeof(float2) * input_size);
    cudaMalloc((void**)&out,   sizeof(float2) * input_size);
    cudaMalloc((void**)&omega, sizeof(float2) * omega_size);

    cudaMemcpy(in,    input,       sizeof(float2) * input_size, cudaMemcpyHostToDevice);
    cudaMemcpy(omega, omegaValues, sizeof(float2) * omega_size, cudaMemcpyHostToDevice);

    fft_stage<<<blocks, threads>>>(in, out, omega, subFftSize, subFftCount, stageIndex);

    cudaDeviceSynchronize();

    cudaMemcpy(output, out, sizeof(float2) * input_size, cudaMemcpyDeviceToHost);

    cudaFree(in);
    cudaFree(out);
    cudaFree(omega);
}
