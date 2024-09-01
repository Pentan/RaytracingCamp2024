# directories
set(PINKYPI_LINEARALGEBRA_DIR linearalgebra)
set(PINKYPI_CORE_DIR pinkycore)
set(PINKYPI_MAIN_DIR pinkypi)

# LinearAlgebra
set(LINEARALGEBRA_SRCS
    ${PINKYPI_LINEARALGEBRA_DIR}/linearalgebra.h
    ${PINKYPI_LINEARALGEBRA_DIR}/constants.h
    ${PINKYPI_LINEARALGEBRA_DIR}/vector3.h
    ${PINKYPI_LINEARALGEBRA_DIR}/vector4.h
    ${PINKYPI_LINEARALGEBRA_DIR}/matrix4.h
    ${PINKYPI_LINEARALGEBRA_DIR}/quaternion.h
    ${PINKYPI_LINEARALGEBRA_DIR}/vector3.inl
    ${PINKYPI_LINEARALGEBRA_DIR}/vector4.inl
    ${PINKYPI_LINEARALGEBRA_DIR}/matrix4.inl
    ${PINKYPI_LINEARALGEBRA_DIR}/quaternion.inl
)
add_library(LinearAlgebra INTERFACE)
# target_include_directories(LinearAlgebra INTERFACE ${PINKYPI_LINEARALGEBRA_DIR})
add_custom_target(LinearAlgebra_h SOURCES ${LINEARALGEBRA_SRCS})

# Core
list(APPEND PINKYPI_CORE_SRCS
    ${PINKYPI_CORE_DIR}/aabb.cc
    ${PINKYPI_CORE_DIR}/config.cc
    ${PINKYPI_CORE_DIR}/framebuffer.cc
    ${PINKYPI_CORE_DIR}/postprocessor.cc
    ${PINKYPI_CORE_DIR}/renderer.cc
    ${PINKYPI_CORE_DIR}/renderscheduler.cc
    ${PINKYPI_CORE_DIR}/scene.cc
    ${PINKYPI_CORE_DIR}/material.cc
    ${PINKYPI_CORE_DIR}/texture.cc
    ${PINKYPI_CORE_DIR}/mesh.cc
    ${PINKYPI_CORE_DIR}/light.cc
    ${PINKYPI_CORE_DIR}/camera.cc
    ${PINKYPI_CORE_DIR}/node.cc
    ${PINKYPI_CORE_DIR}/assetlibrary.cc
    ${PINKYPI_CORE_DIR}/bvh.cc
    ${PINKYPI_CORE_DIR}/animation.cc
    ${PINKYPI_CORE_DIR}/keyframesampler.cc
    ${PINKYPI_CORE_DIR}/skin.cc
    ${PINKYPI_CORE_DIR}/tracablestructure.cc
)
list(APPEND PINKYPI_CORE_SRCS
    ${PINKYPI_CORE_DIR}/pptypes.h
    ${PINKYPI_CORE_DIR}/ray.h
    ${PINKYPI_CORE_DIR}/aabb.h
    ${PINKYPI_CORE_DIR}/config.h
    ${PINKYPI_CORE_DIR}/framebuffer.h
    ${PINKYPI_CORE_DIR}/postprocessor.h
    ${PINKYPI_CORE_DIR}/renderer.h
    ${PINKYPI_CORE_DIR}/renderscheduler.h
    ${PINKYPI_CORE_DIR}/scene.h
    ${PINKYPI_CORE_DIR}/material.h
    ${PINKYPI_CORE_DIR}/texture.h
    ${PINKYPI_CORE_DIR}/mesh.h
    ${PINKYPI_CORE_DIR}/light.h
    ${PINKYPI_CORE_DIR}/camera.h
    ${PINKYPI_CORE_DIR}/node.h
    ${PINKYPI_CORE_DIR}/assetlibrary.h
    ${PINKYPI_CORE_DIR}/bvh.h
    ${PINKYPI_CORE_DIR}/random.h
    ${PINKYPI_CORE_DIR}/animation.h
    ${PINKYPI_CORE_DIR}/keyframesampler.h
    ${PINKYPI_CORE_DIR}/skin.h
    ${PINKYPI_CORE_DIR}/tracablestructure.h
    ${PINKYPI_CORE_DIR}/intersection.h
)

add_library(PinkyCore ${PINKYPI_CORE_SRCS})
target_include_directories(PinkyCore
    PRIVATE
    ${PINKYPI_SOURCE_DIR}
    ${PINKYPI_CORE_DIR}
    ${PINKYPI_THIRDPARTY_DIR}
)
# ${PINKYPI_LINEARALGEBRA_DIR}

# Application
list(APPEND PINLYPI_APP_SRCS
    ${PINKYPI_MAIN_DIR}/sceneloader.cc
)
list(APPEND PINLYPI_APP_SRCS
    ${PINKYPI_MAIN_DIR}/sceneloader.h
)
add_library(PinkyPiApp ${PINLYPI_APP_SRCS})
    target_include_directories(PinkyPiApp
    PRIVATE
    ${PINKYPI_SOURCE_DIR}
    ${PINKYPI_CORE_DIR}
    ${PINKYPI_THIRDPARTY_DIR}
)

# executable settingsset (PLATFORM_DEPEND_LIBS )
set (PLATFORM_DEPEND_LIBS )
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
set (PLATFORM_DEPEND_LIBS winmm.lib)
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

add_executable(PinkyPi
    ${PINKYPI_MAIN_DIR}/main.cc
)
target_link_libraries(PinkyPi PUBLIC
    PinkyPiApp
    PinkyCore
    LinearAlgebra
    ThirdpartyLibs
    ${PLATFORM_DEPEND_LIBS}
)
target_include_directories(PinkyPi PRIVATE
    ${PINKYPI_SOURCE_DIR}
    ${PINKYPI_THIRDPARTY_DIR}
    ${PINKYPI_MAIN_DIR}
)
    # ${PINKYPI_LINEARALGEBRA_DIR}
    # ${PINKYPI_CORE_DIR}
