# tests/CMakeLists.txt

# Config
set(TEST_OUTPUT_DIR "\"${SPCTRNTS_TEST_DIR}/output\"")
set(TEST_DATA_DIR "\"${SPCTRNTS_TEST_DIR}/testdata\"")
configure_file(
    "${SPCTRNTS_TEST_DIR}/testconfig.h.in"
    "${SPCTRNTS_TEST_DIR}/testconfig.h"
)

# Linear Algebra
set(LINEARALGEBRA_TEST_DIR linearalgebraTests)
set(LINEARALGEBRA_TESTS_SRCS
    ${LINEARALGEBRA_TEST_DIR}/Vector3Tests.cc
    ${LINEARALGEBRA_TEST_DIR}/Vector4Tests.cc
    ${LINEARALGEBRA_TEST_DIR}/Matrix4Tests.cc
    ${LINEARALGEBRA_TEST_DIR}/QuaternionTests.cc
)
source_group(linearalgebraTests FILES ${LINEARALGEBRA_TESTS_SRCS})

# Core
set(MAIN_TEST_DIR mainTests)
set(MAIN_TESTS_SRCS 
    ${MAIN_TEST_DIR}/aabbTests.cc
    ${MAIN_TEST_DIR}/framebufferTests.cc
    ${MAIN_TEST_DIR}/randomTests.cc
    ${MAIN_TEST_DIR}/configTests.cc
    ${MAIN_TEST_DIR}/animationTests.cc
    ${MAIN_TEST_DIR}/rayTests.cc
    ${MAIN_TEST_DIR}/bvhTests.cc
    ${MAIN_TEST_DIR}/textureTests.cc
    ${MAIN_TEST_DIR}/materialTests.cc
    ${MAIN_TEST_DIR}/sceneloaderTests.cc
)
source_group(mainTests FILES ${MAIN_TESTS_SRCS})

# lib
set(THIRDPARTY_DOCTEST_DIR ${SPCTRNTS_THIRDPARTY_DIR}/doctest)
set(THIRDPARTY_DOCTEST_SRCS
    ${THIRDPARTY_DOCTEST_DIR}/doctest.h
)
source_group(doctest FILES ${THIRDPARTY_DOCTEST_SRCS})

# exectable
set (PLATFORM_DEPEND_LIBS )
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
set (PLATFORM_DEPEND_LIBS winmm.lib)
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

add_executable(SpectrenotesTests
    testmain.cc
    testsupport.cc
    testsupport.h
    testconfig.h
    ${THIRDPARTY_DOCTEST_SRCS}
    ${LINEARALGEBRA_TESTS_SRCS}
    ${MAIN_TESTS_SRCS}
)
target_link_libraries(SpectrenotesTests PRIVATE
    LinearAlgebra
    SpectrenotesCore
    ThirdpartyLibs
    ${PLATFORM_DEPEND_LIBS}
)
target_include_directories(SpectrenotesTests PRIVATE
    ${SPCTRNTS_SOURCE_DIR}
    ${SPCTRNTS_THIRDPARTY_DIR}
    ${THIRDPARTY_DOCTEST_DIR}
)

# source_group(linearalgebraTests REGULAR_EXPRESSION ${LINEARALGEBRA_TEST_DIR}/*)
