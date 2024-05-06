#include <spectr/real_time_input/RealTimeInputBladeRF.h>

#include <iostream>

namespace spectr::real_time_input {
    RealTimeInputBladeRF::RealTimeInputBladeRF() : device(nullptr) {
        std::cout << "Initializing bladeRF...\n";

        bladerf_devinfo device_info { };

        bladerf_init_devinfo(&device_info);

        int status;

        if ((status = bladerf_open_with_devinfo(&device, &device_info)) != 0) {
            throw std::runtime_error(std::format("Unable to open bladeRF device: {}.", bladerf_strerror(status)));
        }


        if ((status = bladerf_set_frequency(device, BLADERF_CHANNEL_RX(0), frequency)) != 0) {
            throw std::runtime_error(std::format("Unable to set bladeRF frequency: {}.", bladerf_strerror(status)));
        }

        if ((status = bladerf_set_sample_rate(device, BLADERF_CHANNEL_RX(0), sample_rate, nullptr)) != 0) {
            throw std::runtime_error(std::format("Unable to set bladeRF sample rate: {}.", bladerf_strerror(status)));
        }

        if ((status = bladerf_set_bandwidth(device, BLADERF_CHANNEL_RX(0), sample_rate * sizeof(float), nullptr)) != 0) {
            throw std::runtime_error(std::format("Unable to set bladeRF bandwidth: {}.", bladerf_strerror(status)));
        }

        if ((status = bladerf_set_gain(device, BLADERF_CHANNEL_RX(0), 39)) != 0) {
            throw std::runtime_error(std::format("Unable to set bladeRF gain: {}.", bladerf_strerror(status)));
        }

        if ((status = bladerf_sync_config(device,
                                          BLADERF_RX_X1,
                                          BLADERF_FORMAT_SC16_Q11,
                                          buffers,
                                          buffer_size,
                                          transfers,
                                          timeout_ms)) != 0) {
            throw std::runtime_error(std::format("Unable to configure bladeRF RX interface: {}.", bladerf_strerror(status)));
        }

        std::cout << "BladeRF initialized successfully\n"
                     "Starting RX thread\n";

        thread = std::make_unique<std::thread>(RealTimeInputBladeRF::sync_thread,
                                               device,
                                               shutdown.get_future(),
                                               &data);

        std::cout << "BladeRF RX thread initialized.\n";
    }

    RealTimeInputBladeRF::~RealTimeInputBladeRF() {
        shutdown.set_value();
        thread->join();

        if (device != nullptr) {
            bladerf_close(device);
        }
    }

    audio_loader::SignalData RealTimeInputBladeRF::getSignalData() noexcept(true) {
        return getData().toSignalData(downsampled_rate);
    }

    int RealTimeInputBladeRF::getSampleRate() const noexcept(true) {
        return downsampled_rate;
    }
}
