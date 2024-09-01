# tests/CMakeLists.txt

# add_subdirectory(coreTests)
# add_subdirectory(linearalgebraTests)
# add_subdirectory(pinkypiTests)

# Config

set(TEST_OUTPUT_DIR "\"${PINKYPI_TTEST_DIR}/output\"")
set(TEST_DATA_DIR "\"${PINKYPI_TTEST_DIR}/testdata\"")
configure_file(
    "${PINKYPI_TTEST_DIR}/testconfig.h.in"
    "${PINKYPI_TTEST_DIR}/testconfig.h"
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
set(PINKYCORE_TEST_DIR coreTests)
set(PINKYCORE_TESTS_SRCS 
    ${PINKYCORE_TEST_DIR}/aabbTests.cc
    ${PINKYCORE_TEST_DIR}/framebufferTests.cc
    ${PINKYCORE_TEST_DIR}/randomTests.cc
    ${PINKYCORE_TEST_DIR}/configTests.cc
    ${PINKYCORE_TEST_DIR}/animationTests.cc
    ${PINKYCORE_TEST_DIR}/rayTests.cc
    ${PINKYCORE_TEST_DIR}/bvhTests.cc
    ${PINKYCORE_TEST_DIR}/textureTests.cc
    ${PINKYCORE_TEST_DIR}/materialTests.cc
)
source_group(coreTests FILES ${PINKYCORE_TESTS_SRCS})

# App
set(PINKYAPP_TEST_DIR appTests)
set(PINKYAPP_TESTS_SRCS 
    ${PINKYAPP_TEST_DIR}/sceneloaderTests.cc
)
source_group(appTests FILES ${PINKYAPP_TESTS_SRCS})

set(THIRDPARTY_DOCTEST_DIR ${PINKYPI_THIRDPARTY_DIR}/doctest)
set(THIRDPARTY_DOCTEST_SRCS
    ${THIRDPARTY_DOCTEST_DIR}/doctest.h
)
source_group(doctest FILES ${THIRDPARTY_DOCTEST_SRCS})

# exectable
set (PLATFORM_DEPEND_LIBS )
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
set (PLATFORM_DEPEND_LIBS winmm.lib)
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

add_executable(PinkyPiTests
    testmain.cc
    testsupport.cc
    testsupport.h
    testconfig.h
    ${THIRDPARTY_DOCTEST_SRCS}
    ${LINEARALGEBRA_TESTS_SRCS}
    ${PINKYCORE_TESTS_SRCS}
    ${PINKYAPP_TESTS_SRCS}
)
target_link_libraries(PinkyPiTests PRIVATE
    LinearAlgebra
    PinkyCore
    PinkyPiApp
    ThirdpartyLibs
    ${PLATFORM_DEPEND_LIBS}
)
target_include_directories(PinkyPiTests PRIVATE
    ${PINKYPI_SOURCE_DIR}
    ${PINKYPI_THIRDPARTY_DIR}
    ${THIRDPARTY_DOCTEST_DIR}
)

# source_group(linearalgebraTests REGULAR_EXPRESSION ${LINEARALGEBRA_TEST_DIR}/*)
