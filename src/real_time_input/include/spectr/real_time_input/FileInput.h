#pragma once
#include <spectr/real_time_input/RealTimeInput.h>

#pragma once
#include <spectr/real_time_input/RealTimeInput.h>

#include <filesystem>

namespace spectr::real_time_input {
    /**
      * This class reads data from a file.
      *
      * @see SampledData
      */
    class FileInput : public RealTimeInput {
    private:
        audio_loader::SignalData data;
        bool read = false;

    public:
        FileInput(const std::filesystem::path& audioFilePath);

        SampledData getData() noexcept(true);

        audio_loader::SignalData getSignalData() noexcept(true);

        int getSampleRate() const noexcept(true);
    };
}
