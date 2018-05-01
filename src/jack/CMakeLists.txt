set(AUDIOLIB_JACK_SOURCES OutputDspJack.cpp)

find_path(JACK_INCLUDE_DIR NAMES jack/jack.h)
  
find_library(JACK_LIBRARY NAMES jack)

find_package_handle_standard_args(Jack DEFAULT_MSG JACK_LIBRARY JACK_INCLUDE_DIR)

add_library(dpsaudio-jack SHARED ${AUDIOLIB_JACK_SOURCES})
target_link_libraries(dpsaudio-jack ${JACK_LIBRARY})
set_target_properties(dpsaudio-jack PROPERTIES VERSION ${VERSION})
install(TARGETS dpsaudio-jack
    RUNTIME DESTINATION bin OPTIONAL
    ARCHIVE DESTINATION lib OPTIONAL
    LIBRARY DESTINATION lib OPTIONAL
    )
