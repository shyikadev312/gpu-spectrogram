#include <spectr/real_time_input/FileInput.h>
#include <spectr/audio_loader/AudioLoader.h>

namespace spectr::real_time_input {
    FileInput::FileInput(const std::filesystem::path& audioFilePath) : data(audio_loader::AudioLoader::load(audioFilePath)) { }

    SampledData FileInput::getData() noexcept(true) {
        return getSignalData();
    }

    audio_loader::SignalData FileInput::getSignalData() noexcept(true) {
        if (read) {
            return { };
        }

        read = true;

        return data;
    }

    int FileInput::getSampleRate() const noexcept(true) {
        return data.getSampleRate();
    }
}
