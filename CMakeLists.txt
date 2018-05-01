cmake_minimum_required(VERSION 2.6)

project(libdpsaudio)
include(FindPackageHandleStandardArgs)

execute_process(
  COMMAND git describe --always --dirty
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/src ${CMAKE_MODULE_PATH})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS "-Wall -O3 -fPIC")
add_definitions(-D_THREAD_SAFE)
add_definitions(-D_FILE_OFFSET_BITS=64)
add_definitions(-DDEBUG=4)
add_definitions(-DVERSION="\\"${VERSION}\\"")

include_directories(${CMAKE_SOURCE_DIR}/src)

option(ENABLE_FLAC "Enable FLAC audio support." ON)
option(ENABLE_MP3 "Enable MP3 audio support." ON)

option(ENABLE_JACK "Enable JACK audio support." ON)
option(ENABLE_ALSA "Enable ALSA audio support." ON)
option(ENABLE_OSS "Enable OSS audio support." ON)

add_subdirectory(src)