DEFINE_MODULE(spectr.audio_loader)

target_link_libraries(spectr.audio_loader
	spectr.utils
)

add_subdirectory(test)
