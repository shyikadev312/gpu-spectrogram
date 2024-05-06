#pragma once

#include <spectr/real_time_input/RealTimeInput.h>

#include <portaudio.h>

namespace spectr::real_time_input {
    /**
      * This class uses PortAudio to read data from a real-time audio interface.
      * 
      * @see SampledData
      */
    class RealTimeInputPortAudio : public RealTimeInput {
    private:
        PaStream* stream; /**< The underlying PortAudio data stream. */

        bool isRecording; /**< Whether this object is currently recording audio or not. */

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

    public:
        /**
          * Constructs this RealTimeInputPortAudio object by initializing PortAudio and creating
          * a PortAudio data stream.
          * 
          * @throws std::runtime_exception if an error occurred in the initialization of
          *         PortAudio or the PortAudio data stream.
          */
         RealTimeInputPortAudio() noexcept(false);

        /**
          * Destroys this RealTimeInputPortAudio object by terminating PortAudio and the PortAudio
          * data stream.
          * 
          * @throws std::runtime_exception if an error occurred in the termination of
          *         PortAudio or the PortAudio data stream.
          */
        ~RealTimeInputPortAudio() noexcept(false);

        audio_loader::SignalData getSignalData() noexcept(true);

        int getSampleRate() const noexcept(true);
    };
}
