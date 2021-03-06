set(AUDIOLIB_SOURCES    AudioPacket.cpp
                        CircularCache.cpp
                        ComponentAudio.cpp
                        Component.cpp
                        ComponentMonitor.cpp
                        Counter.cpp
                        CounterTrigger.cpp
                        Input.cpp
                        InputFile.cpp
                        Output.cpp
                        OutputDsp.cpp
                        OutputFile.cpp
                        Process.cpp
                        ProcessFader.cpp
                        ProcessLink.cpp
                        ProcessMixer.cpp
                        Thread.cpp
    )

set(AUDIOLIB_HEADERS    AudioPacket.h
                        CircularCache.h
                        ComponentAudio.h
                        Component.h
                        ComponentMonitor.h
                        Counter.h
                        CounterTrigger.h
                        Input.h
                        InputFile.h
                        Output.h
                        OutputDsp.h
                        OutputFile.h
                        Process.h
                        ProcessFader.h
                        ProcessLink.h
                        ProcessMixer.h
                        Thread.h
    )

find_package (Threads)

if (NOT CMAKE_USE_PTHREADS_INIT)
	message(FATAL_ERROR "pthreads support is required for libdpsaudio.")
endif (NOT CMAKE_USE_PTHREADS_INIT)

if (WIN32)
    find_package(dlfcn-win32 REQUIRED)
    set(CMAKE_DL_LIBS dlfcn-win32::dl)
endif (WIN32)

if (NOT APPLE)
    if (NOT CMAKE_DL_LIBS)
        message(FATAL_ERROR "dlfcn support is required for libdpsaudio.")
    endif (NOT CMAKE_DL_LIBS)
endif (NOT APPLE)

add_library(dpsaudio SHARED ${AUDIOLIB_SOURCES})
target_link_libraries (dpsaudio ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(dpsaudio ${CMAKE_DL_LIBS})

set_target_properties(dpsaudio PROPERTIES VERSION ${VERSION})
install(TARGETS dpsaudio
    RUNTIME DESTINATION bin OPTIONAL
    ARCHIVE DESTINATION lib OPTIONAL
    LIBRARY DESTINATION lib OPTIONAL
    )
install(FILES ${AUDIOLIB_HEADERS} DESTINATION include)

if (ENABLE_RAW)
	add_subdirectory(raw)
endif (ENABLE_RAW)
if (ENABLE_OSS)
    add_subdirectory(oss)
endif (ENABLE_OSS)
if (ENABLE_ALSA)
    add_subdirectory(alsa)
endif (ENABLE_ALSA)
if (ENABLE_FLAC)
    add_subdirectory(flac)
endif (ENABLE_FLAC)
if (ENABLE_JACK)
    add_subdirectory(jack)
endif (ENABLE_JACK)
if (ENABLE_MP3)
    add_subdirectory(mp3)
endif (ENABLE_MP3)
if (ENABLE_PULSE)
    add_subdirectory(pulse)
endif (ENABLE_PULSE)
if (ENABLE_RTAUDIO)
    add_subdirectory(rtaudio)
endif (ENABLE_RTAUDIO)
