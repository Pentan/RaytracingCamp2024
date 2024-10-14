# src/CMakeLists.txt
cmake_minimum_required (VERSION 3.20)

project (PETALS)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
add_compile_definitions(_USE_MATH_DEFINES)
add_compile_definitions(NOMINMAX)
endif (CMAKE_SYSTEM_NAME STREQUAL "Windows")

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

set (PETALS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/sources)
set (PETALS_TEST_DIR ${CMAKE_SOURCE_DIR}/tests)
set (PETALS_THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty)

add_subdirectory(sources)
add_subdirectory(tests)
add_subdirectory(thirdparty)
