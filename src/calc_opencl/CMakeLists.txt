DEFINE_MODULE(spectr.calc_opencl)

target_link_libraries(spectr.calc_opencl
	spectr.utils
	OpenCL::OpenCL
	OpenCL::Headers
	OpenCL::HeadersCpp
)

add_subdirectory(test)
