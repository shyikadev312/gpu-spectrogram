#pragma once

#include <spectr/real_time_input/SampledData.h>

#include <portaudio.h>

namespace spectr::real_time_input {
    /**
      * This class uses PortAudio to read data form a real-time audio interface.
      * 
      * @see SampledData
      */
    class RealTimeInput {
    private:
        PaStream* stream; /**< The underlying PortAudio data stream. */

        bool isRecording; /**< Whether this object is currently recording audio or not. */
        SampledData data; /**< All data that has been recorded since the last time data
                             < was retrieved using getData(). */

        /** 
          * The callback function used by PortAudio.
          */
        static int AudioInputCallback(const void* input,
                                      void* output,
                                      unsigned long frameCount,
                                      const PaStreamCallbackTimeInfo* timeInfo,
                                      PaStreamCallbackFlags statusFlags,
                                      void* userData);

        int AudioInputCallback(const void* input,
                               void* output,
                               unsigned long frameCount,
                               const PaStreamCallbackTimeInfo* timeInfo,
                               PaStreamCallbackFlags statusFlags);

    public:
        /**
          * Constructs this RealTimeInput object by initializing PortAudio and creating
          * a PortAudio data stream.
          * 
          * @throws std::runtime_exception if an error occurred in the initialization of
          *         PortAudio or the PortAudio data stream.
          */
         RealTimeInput() noexcept(false);

        /**
          * Destroys this RealTimeInput object by terminating PortAudio and the PortAudio
          * data stream.
          * 
          * @throws std::runtime_exception if an error occurred in the termination of
          *         PortAudio or the PortAudio data stream.
          */
        ~RealTimeInput() noexcept(false);

        /**
          * Begins recording audio using PortAudio.
          * 
          * @throws std::runtime_exception if an error occurred in starting the recording
          */
        void StartRecording() noexcept(false);

        /**
          * Stops recording audio using PortAudio.
          *
          * @throws std::runtime_exception if an error occurred in terminating the recording
          */
        void  StopRecording() noexcept(false);


        /**
          * Gets all the data that has been sampled since the last time this function was
          * called. This function will never return data which has already been read.
          */
        SampledData getData() noexcept(true);
    };
}
