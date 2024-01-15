DEFINE_BENCHMARK_MODULE(spectr.calc_opencl)

target_link_libraries(spectr.calc_opencl.benchmark
	spectr.audio_loader
)
