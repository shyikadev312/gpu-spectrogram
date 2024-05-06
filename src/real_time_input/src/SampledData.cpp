#include <spectr/real_time_input/SampledData.h>

namespace spectr::real_time_input {
    SampledData::SampledData(const SampledData&  data)                : samples(data.samples) { }
    SampledData::SampledData(      SampledData&& data) noexcept(true) : samples(std::move(data.samples)) { }
    SampledData::SampledData(      size_t        size)                : samples(size) { }

    SampledData SampledData::operator+(SampledData& rhs) {
        std::lock_guard<std::mutex> guard { mutex };

        SampledData output { samples.size() + rhs.samples.size() };

        output.samples.insert(output.samples.end(),     samples.begin(),     samples.end());
        output.samples.insert(output.samples.end(), rhs.samples.begin(), rhs.samples.end());

        return output;
    }

    SampledData& SampledData::operator+=(SampledData& rhs) {
        std::lock_guard<std::mutex> guard{ mutex };

        samples.resize(samples.size() + rhs.samples.size());

        samples.insert(samples.end(), rhs.samples.begin(), rhs.samples.end());

        return *this;
    }
}
