DEFINE_MODULE(spectr.calc_cpu)

target_link_libraries(spectr.calc_cpu
	spectr.utils
)

add_subdirectory(test)
