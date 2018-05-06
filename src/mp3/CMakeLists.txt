find_path(MPG123_INCLUDE_DIR mpg123.h)

if(WIN32)
    find_library(MPG123_LIBRARY NAMES libmpg123)
else(WIN32)
    find_library(MPG123_LIBRARY NAMES mpg123 mpg123-0)
endif(WIN32)

find_package_handle_standard_args(MPG123 DEFAULT_MSG MPG123_LIBRARY MPG123_INCLUDE_DIR)

if (NOT MPG123_FOUND)
	message("mpg123 Library not found - not building.")
endif (NOT MPG123_FOUND)

if (MPG123_FOUND)
	set(AUDIOLIB_MP3_SOURCES InputMp3.cpp)
	add_library(dpsaudio-mp3 SHARED ${AUDIOLIB_MP3_SOURCES})
	target_link_libraries(dpsaudio-mp3 ${MPG123_LIBRARY})
	set_target_properties(dpsaudio-mp3 PROPERTIES VERSION ${VERSION})
	install(TARGETS dpsaudio-mp3
		RUNTIME DESTINATION bin OPTIONAL
		ARCHIVE DESTINATION lib OPTIONAL
		LIBRARY DESTINATION lib OPTIONAL
		)
endif (MPG123_FOUND)
