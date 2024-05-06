#include <spectr/real_time_input/SampledData.h>

namespace spectr::real_time_input {
    SampledData::SampledData(const SampledData&  data)                : samples(data.samples) { }
    SampledData::SampledData(      SampledData&& data) noexcept(true) : samples(std::move(data.samples)) { }
    SampledData::SampledData(      size_t        size)                : samples(size) { }

    SampledData::SampledData(audio_loader::SignalData data) {
        if (data.getChannelCount() == 0) {
            return;
        }

        if (!std::holds_alternative<audio_loader::SampleDataFloat>(data.getChannelSampleData(0))) {
            throw std::runtime_error("Cannot create SampledData from non float SinglaData.");
        }

        samples = std::get<audio_loader::SampleDataFloat>(data.getChannelSampleData(0));
    }

    SampledData SampledData::operator+(SampledData& rhs) {
        std::lock_guard<std::mutex> guard { mutex };

        SampledData output { samples.size() + rhs.samples.size() };

        output.samples.insert(output.samples.end(),     samples.begin(),     samples.end());
        output.samples.insert(output.samples.end(), rhs.samples.begin(), rhs.samples.end());

        return output;
    }

    SampledData& SampledData::operator+=(SampledData& rhs) {
        std::lock_guard<std::mutex> guard { mutex };

        samples.resize(samples.size() + rhs.samples.size());

        samples.insert(samples.end(), rhs.samples.begin(), rhs.samples.end());

        return *this;
    }

    audio_loader::SignalData SampledData::toSignalData(size_t sampleRate) const {
        std::vector<audio_loader::SampleDataVariant> data;

        data.push_back(samples);

        return audio_loader::SignalData { sampleRate, data };
    }
}
