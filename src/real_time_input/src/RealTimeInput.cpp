#include <spectr/real_time_input/RealTimeInput.h>

#include <stdexcept>
#include <format>

constexpr auto sampleFormat    = paFloat32;
constexpr auto sampleRate      = 44100;
constexpr auto framesPerBuffer = 256;

namespace spectr::real_time_input {
    int RealTimeInput::AudioInputCallback(const void* input,
                                          void* output,
                                          unsigned long frameCount,
                                          const PaStreamCallbackTimeInfo* timeInfo,
                                          PaStreamCallbackFlags statusFlags,
                                          void* userData) {
        return ((RealTimeInput*)userData)->AudioInputCallback(input,
                                                              output,
                                                              frameCount,
                                                              timeInfo,
                                                              statusFlags);
    }

    int RealTimeInput::AudioInputCallback(const void* input,
                                          void* output,
                                          unsigned long frameCount,
                                          const PaStreamCallbackTimeInfo* timeInfo,
                                          PaStreamCallbackFlags statusFlags) {
        SampledData newData {
            (float*)output,
            frameCount
        };

        data += newData;

        return PaStreamCallbackResult::paContinue;
    }

    RealTimeInput::RealTimeInput() noexcept(false) : stream(nullptr), isRecording(false), data({ nullptr, 0 }) {
        auto error = Pa_Initialize();

        if (error != paNoError) {
            throw std::runtime_error(std::format("Could not initialize PortAudio: {}", Pa_GetErrorText(error)));
        }

        error = Pa_OpenDefaultStream(&stream,
                                     1,
                                     0,
                                     sampleFormat,
                                     sampleRate,
                                     framesPerBuffer,
                                     AudioInputCallback,
                                     this);

        if (error != paNoError) {
            throw std::runtime_error(std::format("Could not initialize PortAudio input stream: {}", Pa_GetErrorText(error)));
        }
    }

    RealTimeInput::~RealTimeInput() noexcept(false) {
        std::string errors = "";

        try {
            StopRecording();
        } catch (std::runtime_error& e) {
            errors = e.what();
        }

        auto error = Pa_Terminate();

        if (error != paNoError) {
            if (errors != "") errors += "\n";
            
            errors += std::format("Could not terminate PortAudio: {}", Pa_GetErrorText(error));
        }

        if (errors != "") {
            throw std::runtime_error(errors);
        }
    }

    void RealTimeInput::StartRecording() noexcept(false) {
        if (!isRecording) {
            auto error = Pa_StartStream(stream);

            if (error != paNoError) {
                throw std::runtime_error(std::format("Could not start recording: {}", Pa_GetErrorText(error)));
            }

            isRecording = true;
        }
    }

    void RealTimeInput::StopRecording() noexcept(false) {
        if (isRecording) {
            auto error = Pa_StopStream(stream);

            if (error != paNoError) {
                throw std::runtime_error(std::format("Could not stop recording: {}", Pa_GetErrorText(error)));
            }

            isRecording = false;
        }
    }

    SampledData RealTimeInput::getData() noexcept(true) {
        SampledData ret = data;

        data.samples     = nullptr;
        data.sampleCount = 0;

        return ret;
    }
}
