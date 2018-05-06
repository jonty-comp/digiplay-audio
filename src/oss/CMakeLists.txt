find_path(OSS_INCLUDE_DIR sys/soundcard.h)

find_package_handle_standard_args(OSS DEFAULT_MSG OSS_INCLUDE_DIR)

if (NOT OSS_FOUND)
	message ("OSS Library not found - not building.")
endif (NOT OSS_FOUND)

if (OSS_FOUND)
	set(AUDIOLIB_OSS_SOURCES OutputDspOss.cpp)
	add_library(dpsaudio-oss SHARED ${AUDIOLIB_OSS_SOURCES})
	set_target_properties(dpsaudio-oss PROPERTIES VERSION ${VERSION})
	install(TARGETS dpsaudio-oss
		RUNTIME DESTINATION bin OPTIONAL
		ARCHIVE DESTINATION lib OPTIONAL
		LIBRARY DESTINATION lib OPTIONAL
		)
endif (OSS_FOUND)
