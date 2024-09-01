# src/CMakeLists.txt
cmake_minimum_required (VERSION 3.8)

option(PINKYPI_NO_STD_FILESYSTEM "don't use c++ filesystem" OFF)
message(STATUS "PINKYPI_NO_STD_FILESYSTEM ${PINKYPI_NO_STD_FILESYSTEM}")

project (PinkyPi)

if (PINKYPI_NO_STD_FILESYSTEM)
add_compile_definitions(PINKYPI_NO_STD_FILESYSTEM)
endif (PINKYPI_NO_STD_FILESYSTEM)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
add_compile_definitions(_USE_MATH_DEFINES)
add_compile_definitions(NOMINMAX)
endif (CMAKE_SYSTEM_NAME STREQUAL "Windows")

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

set (PINKYPI_SOURCE_DIR ${CMAKE_SOURCE_DIR}/sources)
set (PINKYPI_TTEST_DIR ${CMAKE_SOURCE_DIR}/tests)
set (PINKYPI_THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty)

add_subdirectory(sources)
add_subdirectory(tests)
add_subdirectory(thirdparty)
