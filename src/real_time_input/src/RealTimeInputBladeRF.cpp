#include <spectr/real_time_input/RealTimeInputBladeRF.h>

namespace spectr::real_time_input {
    RealTimeInputBladeRF::RealTimeInputBladeRF() : device(nullptr) {
        bladerf_devinfo device_info { };

        bladerf_init_devinfo(&device_info);

        int status;

        if ((status = bladerf_open_with_devinfo(&device, &device_info)) != 0) {
            throw std::runtime_error(std::format("Unable to open bladeRF device: {}.", bladerf_strerror(status)));
        }


        if ((status = bladerf_set_frequency(device, BLADERF_CHANNEL_RX(0), 910000000)) != 0) {
            throw std::runtime_error(std::format("Unable to set bladeRF frequency: {}.", bladerf_strerror(status)));
        }

        if ((status = bladerf_set_sample_rate(device, BLADERF_CHANNEL_RX(0), 300000, nullptr)) != 0) {
            throw std::runtime_error(std::format("Unable to set bladeRF sample rate: {}.", bladerf_strerror(status)));
        }

        if ((status = bladerf_set_bandwidth(device, BLADERF_CHANNEL_RX(0), 2000000, nullptr)) != 0) {
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

        thread = std::make_unique<std::thread>(RealTimeInputBladeRF::sync_thread,
                                               device,
                                               shutdown.get_future(),
                                               data);
    }

    RealTimeInputBladeRF::~RealTimeInputBladeRF() {
        shutdown.set_value();
        thread->join();

        if (device != nullptr) {
            bladerf_close(device);
        }
    }
}
