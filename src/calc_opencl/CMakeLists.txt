DEFINE_MODULE(spectr.calc_opencl)

target_link_libraries(spectr.calc_opencl
	spectr.utils
	spectr.calc_cpu
	spectr.render_gl
	OpenCL::OpenCL
	OpenCL::Headers
	OpenCL::HeadersCpp
)

add_subdirectory(test)
add_subdirectory(benchmark)
