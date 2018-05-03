set(AUDIOLIB_MP3_SOURCES InputMp3.cpp)

find_path(MPG123_INCLUDE_DIR mpg123.h)

find_library(MPG123_LIBRARIES NAMES mpg123 mpg123-0)

find_package_handle_standard_args(MPG123 DEFAULT_MSG MPG123_LIBRARIES MPG123_INCLUDE_DIR)

add_library(dpsaudio-mp3 SHARED ${AUDIOLIB_MP3_SOURCES})
target_link_libraries(dpsaudio-mp3 ${MPG123_LIBRARIES})
target_link_libraries(dpsaudio-mp3 dpsaudio)
set_target_properties(dpsaudio-mp3 PROPERTIES VERSION ${VERSION})
install(TARGETS dpsaudio-mp3
    RUNTIME DESTINATION bin OPTIONAL
    ARCHIVE DESTINATION lib OPTIONAL
    LIBRARY DESTINATION lib OPTIONAL
    )
