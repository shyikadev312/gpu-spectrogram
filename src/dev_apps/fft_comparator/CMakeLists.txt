DEFINE_EXECUTABLE_MODULE(spectr.dev_apps.fft_comparator)

target_link_libraries(spectr.dev_apps.fft_comparator
	spectr.audio_loader
	spectr.calc_opencl
	spectr.calc_cpu
)
