# src/CMakeLists.txt
cmake_minimum_required (VERSION 3.20)

# option(SPCTRNTS_NO_STD_FILESYSTEM "don't use c++ filesystem" OFF)
# message(STATUS "SPCTRNTS_NO_STD_FILESYSTEM ${SPCTRNTS_NO_STD_FILESYSTEM}")

project (SPECTRENOTES)

# if (SPCTRNTS_NO_STD_FILESYSTEM)
# add_compile_definitions(SPCTRNTS_NO_STD_FILESYSTEM)
# endif (SPCTRNTS_NO_STD_FILESYSTEM)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
add_compile_definitions(_USE_MATH_DEFINES)
add_compile_definitions(NOMINMAX)
endif (CMAKE_SYSTEM_NAME STREQUAL "Windows")

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

set (SPCTRNTS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/sources)
set (SPCTRNTS_TEST_DIR ${CMAKE_SOURCE_DIR}/tests)
set (SPCTRNTS_THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty)

add_subdirectory(sources)
add_subdirectory(tests)
add_subdirectory(thirdparty)
