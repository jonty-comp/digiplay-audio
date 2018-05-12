find_path(RTAUDIO_INCLUDE_DIR NAMES rtaudio/RtAudio.h)

if (NOT RTAUDIO_INCLUDE_DIR)
	find_path(RTAUDIO_INCLUDE_DIR NAMES RtAudio.h)
	add_definitions(-DALTERNATE_RTAUDIO_INCLUDE_DIR)
endif (NOT RTAUDIO_INCLUDE_DIR)
  
find_library(RTAUDIO_LIBRARY NAMES rtaudio)

find_package_handle_standard_args(RtAudio DEFAULT_MSG RTAUDIO_LIBRARY RTAUDIO_INCLUDE_DIR)

if (NOT RtAudio_FOUND)
	message("RtAudio Library not found - not building.")
endif (NOT RtAudio_FOUND)

if (RtAudio_FOUND)
	set(AUDIOLIB_RTAUDIO_SOURCES OutputDspRtAudio.cpp)
	add_library(dpsaudio-rtaudio SHARED ${AUDIOLIB_RTAUDIO_SOURCES})
	target_link_libraries(dpsaudio-rtaudio ${RTAUDIO_LIBRARY})
	target_link_libraries(dpsaudio-rtaudio dpsaudio)
	set_target_properties(dpsaudio-rtaudio PROPERTIES VERSION ${VERSION})
	install(TARGETS dpsaudio-rtaudio
		RUNTIME DESTINATION bin OPTIONAL
		ARCHIVE DESTINATION lib OPTIONAL
		LIBRARY DESTINATION lib OPTIONAL
		)
endif (RtAudio_FOUND)