DEFINE_EXECUTABLE_MODULE(spectr.desktop_app)

target_link_libraries(spectr.desktop_app
	spectr.utils
	spectr.audio_loader
	spectr.calc_cpu
	spectr.calc_opencl
	spectr.render_gl
)
