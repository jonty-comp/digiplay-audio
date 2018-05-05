set(AUDIOLIB_FLAC_SOURCES InputFlac.cpp)

find_path(FLACPP_INCLUDE_DIR
		NAMES FLAC++/all.h FLAC++/decoder.h FLAC++/encoder.h FLAC++/export.h FLAC++/metadata.h)

find_library(FLACPP_LIBRARY NAMES FLAC++)

find_package_handle_standard_args(FLACPP DEFAULT_MSG FLACPP_LIBRARY FLACPP_INCLUDE_DIR)

add_library(dpsaudio-flac SHARED ${AUDIOLIB_FLAC_SOURCES})
target_link_libraries(dpsaudio-flac ${FLACPP_LIBRARY})
target_link_libraries(dpsaudio-flac dpsaudio)

set_target_properties(dpsaudio-flac PROPERTIES VERSION ${VERSION})
install(TARGETS dpsaudio-flac
    RUNTIME DESTINATION bin OPTIONAL
    ARCHIVE DESTINATION lib OPTIONAL
    LIBRARY DESTINATION lib OPTIONAL
    )
