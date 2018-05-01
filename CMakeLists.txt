cmake_minimum_required(VERSION 2.6)

project(libdpsaudio)
set(VERSION 0.99)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/dist CACHE PATH "" FORCE)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/src ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_FLAGS "-Wall -O3 -fPIC")
add_definitions(-D_THREAD_SAFE)
add_definitions(-D_FILE_OFFSET_BITS=64)
add_definitions(-DDEBUG=4)
add_definitions(-DVERSION="\\"${VERSION}\\"")

include_directories(${CMAKE_SOURCE_DIR}/src)

include(CMakeDependentOption)

CMAKE_DEPENDENT_OPTION(ENABLE_FLAC "Enable FLAC audio support." OFF
                        "BUILD_AUDIO_LIBRARY" OFF)
CMAKE_DEPENDENT_OPTION(ENABLE_MP3 "Enable MP3 audio support." OFF
                        "BUILD_AUDIO_LIBRARY" OFF)

CMAKE_DEPENDENT_OPTION(ENABLE_JACK "Enable JACK audio support." OFF
                        "BUILD_AUDIO_LIBRARY" OFF)
CMAKE_DEPENDENT_OPTION(ENABLE_ALSA "Enable ALSA audio support." ON
                        "BUILD_AUDIO_LIBRARY" OFF)
CMAKE_DEPENDENT_OPTION(ENABLE_OSS "Enable OSS audio support." ON
                        "BUILD_AUDIO_LIBRARY" OFF)

add_subdirectory(src)