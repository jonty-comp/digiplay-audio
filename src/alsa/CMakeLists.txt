find_package(ALSA)

if (NOT ALSA_FOUND)
	message("ALSA Library not found - not building.")
endif (NOT ALSA_FOUND)

if (ALSA_FOUND)
	set(AUDIOLIB_ALSA_SOURCES OutputDspAlsa.cpp)
	add_library(dpsaudio-alsa SHARED ${AUDIOLIB_ALSA_SOURCES})
	target_link_libraries(dpsaudio-alsa ${ALSA_LIBRARY})
	set_target_properties(dpsaudio-alsa PROPERTIES VERSION ${VERSION})
	install(TARGETS dpsaudio-alsa
		RUNTIME DESTINATION bin OPTIONAL
		ARCHIVE DESTINATION lib OPTIONAL
		LIBRARY DESTINATION lib OPTIONAL
		)
endif (ALSA_FOUND)
