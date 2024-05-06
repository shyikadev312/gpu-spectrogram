#include <spectr/real_time_input/RealTimeInput.h>

namespace spectr::real_time_input {
    SampledData RealTimeInput::getData() noexcept(true) {
        std::lock_guard<std::mutex> guard { data.mutex };

        SampledData ret = data;

        data.samples.clear();

        return ret;
    }
}
