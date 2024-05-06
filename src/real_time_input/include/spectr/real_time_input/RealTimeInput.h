#pragma once

#include <spectr/real_time_input/SampledData.h>

namespace spectr::real_time_input {
    class RealTimeInput {
    protected:
        SampledData data; /**< All data that has been recorded since the last time data
                             < was retrieved using getData(). */

    public:
        /**
          * Gets all the data that has been sampled since the last time this function was
          * called. This function will never return data which has already been read.
          */
        SampledData getData() noexcept(true);

        /**
          * Gets all the data that has been sampled since the last time this function was
          * called. This function will never return data which has already been read.
          */
        virtual audio_loader::SignalData getSignalData() noexcept(true) = 0;

        virtual int getSampleRate() const noexcept(true) = 0;

        friend class SampledData;
    };
}
