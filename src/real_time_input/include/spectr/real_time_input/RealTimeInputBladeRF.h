#pragma once
#include <spectr/real_time_input/RealTimeInput.h>

#include <libbladeRF.h>

#include <memory>
#include <thread>
#include <future>
#include <format>
#include <stdexcept>

namespace spectr::real_time_input {
    /**
      * This class reads data from a BladeRF interface.
      *
      * @see SampledData
      */
    class RealTimeInputBladeRF : public RealTimeInput {
    private:
        static constexpr int    buffers           = 16;
        static constexpr int    buffer_size       = 65536;
        static constexpr int    transfers         = 8;
        static constexpr int    timeout_ms        = 3500;
        static constexpr int    sample_rate       = 48000000;
        static constexpr int    downsampled_rate  = sample_rate / 400;
        static constexpr size_t sleep_ms          = 50;
        static constexpr size_t total_buffer_size = sample_rate / 1000 * sleep_ms * 32 / 8;
        static constexpr size_t frequency         = 2450000000;

        bladerf*                     device;
        std::unique_ptr<std::thread> thread;
        std::promise<void>           shutdown;

        static void sync_thread(bladerf* device,
                                std::future<void> shutdown,
                                SampledData* data) {
            constexpr size_t samples_per_downsampled = sample_rate / downsampled_rate;

            int status;

            if ((status = bladerf_enable_module(device, BLADERF_RX, true)) != 0) {
                throw std::runtime_error(std::format("Unable to enable BladeRF RX: {}.", bladerf_strerror(status)));
            }

            while (shutdown.wait_for(std::chrono::milliseconds(sleep_ms)) == std::future_status::timeout) {
                std::vector<int16_t> buffer { };

                buffer.resize(total_buffer_size * 2);

                if ((status = bladerf_sync_rx(device, buffer.data(), total_buffer_size, nullptr, timeout_ms)) != 0) {
                    throw std::runtime_error(std::format("Unable to receive data from BladeRF: {}.", bladerf_strerror(status)));
                }

                SampledData newData { };
                
                // Just a quick and dirty test for converting data to the right format, will be replaced asap
                for (size_t i = 0; i < downsampled_rate / 1000 * 50; i++) {
                    size_t id = i * samples_per_downsampled * 2;

                    double val = 0;

                    for (size_t j = 0; j < samples_per_downsampled; j++) {
                        val += buffer[id + j * 2];
                    }

                    val /= samples_per_downsampled;

                    newData.samples.push_back(val);
                }

                // newData.samples.insert(newData.samples.end(), buffer.begin(), buffer.end());

                *data += newData;
            }

            bladerf_enable_module(device, BLADERF_RX, false);
        }

    public:
        RealTimeInputBladeRF();
        ~RealTimeInputBladeRF();

        audio_loader::SignalData getSignalData() noexcept(true);

        int getSampleRate() const noexcept(true);
    };
}
