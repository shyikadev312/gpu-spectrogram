#pragma once

#include <vector>
#include <mutex>

namespace spectr::real_time_input {
    /**
      * Simple class for storing data sampled from real-time input devices.
      * 
      * This class comes with a few simple helper functions to store additional data into
      * an existing SampledData instance.
      */
    class SampledData {
    protected:
        std::mutex mutex;           /**< Lock used for syncrhonizing across threads */

    public:
        std::vector<float> samples; /**< Array which stores the sampled data */

        SampledData() = default;
        SampledData(const SampledData&  data);
        SampledData(      SampledData&& data) noexcept(true);
        SampledData(size_t size);

        /**
          * Creates a new instance of SampledData containing the data from both objects.
          */
        SampledData  operator+ (SampledData& rhs);

        /**
          * Appends the data stored within rhs to this object.
          */
        SampledData& operator+=(SampledData& rhs);

        friend class RealTimeInput;
    };
}
