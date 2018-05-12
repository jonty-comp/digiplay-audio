find_path(JACK_INCLUDE_DIR NAMES jack/jack.h)

if(WIN32)
    find_library(JACK_LIBRARY NAMES jack2)
else(WIN32)
    find_library(JACK_LIBRARY NAMES jack)
endif(WIN32)
    
find_package_handle_standard_args(Jack DEFAULT_MSG JACK_LIBRARY JACK_INCLUDE_DIR)

if (NOT Jack_FOUND)
	message ("JACK Library not found - not building.")
endif (NOT Jack_FOUND)

if (Jack_FOUND)
	set(AUDIOLIB_JACK_SOURCES OutputDspJack.cpp)
	add_library(dpsaudio-jack SHARED ${AUDIOLIB_JACK_SOURCES})
	target_link_libraries(dpsaudio-jack ${JACK_LIBRARY})
	target_link_libraries(dpsaudio-jack dpsaudio)
	set_target_properties(dpsaudio-jack PROPERTIES VERSION ${VERSION})
	install(TARGETS dpsaudio-jack
		RUNTIME DESTINATION bin OPTIONAL
		ARCHIVE DESTINATION lib OPTIONAL
		LIBRARY DESTINATION lib OPTIONAL
		)
endif (Jack_FOUND)
