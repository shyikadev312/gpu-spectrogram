#pragma once

namespace spectr::real_time_input {
    /**
      * Simple structure for storing data sampled from real-time input devices.
      * 
      * This structure comes with a few simple helper functions to store additional data into
      * an existing SampledData instance.
      */
    struct SampledData {
        float* samples;     /**< Array which stores the sampled data */
        size_t sampleCount; /**< The amount of stored samples */

        /**
          * Creates a new instance of SampledData containing the data from both objects.
          */
        SampledData  operator+ (SampledData& rhs);

        /**
          * Appends the data stored within rhs to this object.
          */
        SampledData& operator+=(SampledData& rhs);
    };
}
