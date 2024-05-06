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
      * This class reads data form a BladeRF interface.
      *
      * @see SampledData
      */
    class RealTimeInputBladeRF : public RealTimeInput {
    private:
        static constexpr int buffers     = 16;
        static constexpr int buffer_size = 8192;
        static constexpr int transfers   = 8;
        static constexpr int timeout_ms  = 3500;

        bladerf*                     device;
        std::unique_ptr<std::thread> thread;
        std::promise<void>           shutdown;

        static void sync_thread(bladerf* device,
                                std::future<void> shutdown,
                                SampledData data) {
            while (shutdown.wait_for(std::chrono::milliseconds(50)) == std::future_status::timeout) {
                std::vector<float> buffer { buffer_size };

                int status;

                if ((status = bladerf_enable_module(device, BLADERF_RX, true)) != 0) {
                    throw std::runtime_error(std::format("Unable to set enable BladeRF RX: {}.", bladerf_strerror(status)));
                }

                if ((status = bladerf_sync_rx(device, buffer.data(), buffer_size, nullptr, timeout_ms)) != 0) {
                    throw std::runtime_error(std::format("Unable to receive data from BladeRF: {}.", bladerf_strerror(status)));
                }

                SampledData newData { buffer_size };
                
                newData.samples.insert(newData.samples.end(), buffer.begin(), buffer.end());

                data += newData;
            }

            bladerf_enable_module(device, BLADERF_RX, false);
        }

    public:
        RealTimeInputBladeRF();
        ~RealTimeInputBladeRF();
    };
}
