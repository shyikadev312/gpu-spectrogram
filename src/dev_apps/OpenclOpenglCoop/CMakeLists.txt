DEFINE_EXECUTABLE_MODULE(spectr.dev_apps.opencl_opengl_coop)

target_link_libraries(spectr.dev_apps.opencl_opengl_coop
	spectr.calc_opencl
        spectr.render_gl
)
