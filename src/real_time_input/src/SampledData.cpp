#include <spectr/real_time_input/SampledData.h>

#include <cstring>

namespace spectr::real_time_input {
    SampledData SampledData::operator+(SampledData& rhs) {
        if (!samples && !rhs.samples) return { };

        SampledData output {
            new float[sampleCount + rhs.sampleCount],
            sampleCount + rhs.sampleCount
        };

        if (    samples) memcpy( output.samples,                  samples,     sampleCount * sizeof(float));
        if (rhs.samples) memcpy(&output.samples[sampleCount], rhs.samples, rhs.sampleCount * sizeof(float));

        return output;
    }

    SampledData& SampledData::operator+=(SampledData& rhs) {
        if (!rhs.samples) return *this;

        auto oldSampleCount = sampleCount;

        sampleCount += rhs.sampleCount;

        float* data = new float[sampleCount];

        if (    samples) memcpy( data,                     samples,  oldSampleCount * sizeof(float));
        if (rhs.samples) memcpy(&data[oldSampleCount], rhs.samples, rhs.sampleCount * sizeof(float));

        if (samples) delete[] samples;

        samples = data;

        return *this;
    }
}
