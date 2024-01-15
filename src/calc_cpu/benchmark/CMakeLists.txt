DEFINE_BENCHMARK_MODULE(spectr.calc_cpu)

target_link_libraries(spectr.calc_cpu.benchmark
	spectr.audio_loader
)
