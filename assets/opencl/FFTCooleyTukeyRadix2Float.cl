uint bitReverse(uint v) // TODO compare performance with lookup table
{
   // swap odd and even bits
   v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
   // swap consecutive pairs
   v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
   // swap nibbles
   v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
   // swap bytes
   v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
   // swap 2-byte long pairs
   v = ( v >> 16             ) | ( v               << 16);

   v = v >> BIT_REVERSE_SHIFT_VALUE;

   return v;
}

// TODO compact convert to/from real values

float2 complexMultiply(float2 a, float2 b)
{
   return (float2)(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}

__kernel void bit_reverse_permutation(
   __global float2* input,
   __global float2* output
   )
{
   uint globalId = get_global_id(0);
   uint i1 = globalId;
   uint i2 = bitReverse(globalId);
   if (globalId < FFT_SIZE) //?
   {
      output[i2] = input[i1];
   }
}

__kernel void fft_stage(
   __global float2* input,
   __global float2* output,
   __global float2* omegaValues,
   uint subFftSize,
   uint subFftCount,
   uint stageIndex
   )
{
   const uint subFftIndex = get_global_id(0);
   const uint subFftElementIndex = get_global_id(1);
   const uint subFftHalfSize = subFftSize / 2;

   const uint index1 = subFftIndex * subFftSize + subFftElementIndex;
   const uint index2 = index1 + subFftHalfSize;

   const float2 input1 = input[index1];
   const float2 input2 = input[index2];

   const float2 omegaK = omegaValues[subFftElementIndex];
   const float2 y1 = input1 + complexMultiply(omegaK, input2);
   const float2 y2 = input1 - complexMultiply(omegaK, input2);

   output[index1] = y1;
   output[index2] = y2;
}

__kernel void calculate_magnitudes(
   __global float2* fft,
   __global float* magnitudes
   )
{
   const uint i = get_global_id(0);
   magnitudes[i] = 2 * sqrt((float)(pow(fft[i].x, 2) + pow(fft[i].y, 2)));
}

__kernel void find_max(
   __global float* values,
   __local float* temp,
   __global float* output
   )
{
   const size_t globalId = get_global_id(0);
   const size_t localId = get_local_id(0);
   const size_t localSize = get_local_size(0);

   temp[localId] = values[globalId];
   barrier(CLK_LOCAL_MEM_FENCE);

   for (int i = localSize >> 1; i > 0; i >>= 1)
   {
      if (localId < i)
      {
         if (temp[localId] < temp[localId + i])
         {
            temp[localId] = temp[localId + i];
         }

         barrier(CLK_LOCAL_MEM_FENCE);
      }
   }

   if (localId == 0)
   {
      output[get_group_id(0)] = temp[0];
   }
}
