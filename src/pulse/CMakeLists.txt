set(AUDIOLIB_PULSE_SOURCES OutputDspPulseAudio.cpp)

find_path(PULSE_INCLUDE_DIR NAMES pulse/simple.h)
  
find_library(PULSE_LIBRARY NAMES pulse-simple)

find_package_handle_standard_args(Pulse DEFAULT_MSG PULSE_LIBRARY PULSE_INCLUDE_DIR)

add_library(dpsaudio-pulse SHARED ${AUDIOLIB_PULSE_SOURCES})
target_link_libraries(dpsaudio-pulse ${PULSE_LIBRARY})
set_target_properties(dpsaudio-pulse PROPERTIES VERSION ${VERSION})
install(TARGETS dpsaudio-pulse
    RUNTIME DESTINATION bin OPTIONAL
    ARCHIVE DESTINATION lib OPTIONAL
    LIBRARY DESTINATION lib OPTIONAL
    )
