#pragma once
#include <spectr/real_time_input/RealTimeInput.h>

#include <libbladeRF.h>

#include <memory>
#include <thread>
#include <future>
#include <format>
#include <stdexcept>

// Only works with positive integer exponents but it's fine for this
template<typename T>
consteval T const_pow(T base, T exponent) {
    if (exponent == 1) {
        return base;
    }

    return base * const_pow(base, exponent - 1);
}

namespace spectr::real_time_input {
    /**
      * This class reads data from a BladeRF interface.
      *
      * @see SampledData
      */
    class RealTimeInputBladeRF : public RealTimeInput {
    private:
        static constexpr int    buffers           = 16;
        static constexpr int    buffer_size       = const_pow(2, 16);;
        static constexpr int    transfers         = 8;
        static constexpr int    timeout_ms        = 2500;
        static constexpr int    sample_rate       = const_pow(2, 22);
        static constexpr size_t sleep_ms          = 1;
        static constexpr size_t total_buffer_size = buffer_size / buffers;
        static constexpr size_t frequency         = 2450000000;

        bladerf*                     device;
        std::unique_ptr<std::thread> thread;
        std::promise<void>           shutdown;

        static void sync_thread(bladerf* device,
                                std::future<void> shutdown,
                                SampledData* data) {
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

                newData.addSamples(buffer);

                *data += newData;
            }

            bladerf_enable_module(device, BLADERF_RX, false);
        }

    public:
        static constexpr size_t min_frequency = 50000000;
        static constexpr size_t max_frequency = 6000000000;
        static constexpr size_t min_sample_rate = const_pow(2, 15);
        static constexpr size_t max_sample_rate = const_pow(2, 22);
        RealTimeInputBladeRF();
        ~RealTimeInputBladeRF();

        audio_loader::SignalData getSignalData() noexcept(true);
        
        virtual size_t getFrequencyOffset() const noexcept;

        int getSampleRate() const noexcept(true);
    };
}
