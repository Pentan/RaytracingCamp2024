# directories
set(SPCTRNTS_LINEARALGEBRA_DIR linearalgebra)
set(SPCTRNTS_MAIN_DIR spectrenotes)

# LinearAlgebra
set(LINEARALGEBRA_SRCS
    ${SPCTRNTS_LINEARALGEBRA_DIR}/linearalgebra.h
    ${SPCTRNTS_LINEARALGEBRA_DIR}/constants.h
    ${SPCTRNTS_LINEARALGEBRA_DIR}/vector3.h
    ${SPCTRNTS_LINEARALGEBRA_DIR}/vector4.h
    ${SPCTRNTS_LINEARALGEBRA_DIR}/matrix4.h
    ${SPCTRNTS_LINEARALGEBRA_DIR}/quaternion.h
    ${SPCTRNTS_LINEARALGEBRA_DIR}/vector3.inl
    ${SPCTRNTS_LINEARALGEBRA_DIR}/vector4.inl
    ${SPCTRNTS_LINEARALGEBRA_DIR}/matrix4.inl
    ${SPCTRNTS_LINEARALGEBRA_DIR}/quaternion.inl
)
add_library(LinearAlgebra INTERFACE)
add_custom_target(LinearAlgebra_h SOURCES ${LINEARALGEBRA_SRCS})

# Core
list(APPEND SPCTRNTS_CORE_SRCS
    ${SPCTRNTS_MAIN_DIR}/aabb.cc
    ${SPCTRNTS_MAIN_DIR}/config.cc
    ${SPCTRNTS_MAIN_DIR}/framebuffer.cc
    ${SPCTRNTS_MAIN_DIR}/postprocessor.cc
    ${SPCTRNTS_MAIN_DIR}/renderer.cc
    ${SPCTRNTS_MAIN_DIR}/renderscheduler.cc
    ${SPCTRNTS_MAIN_DIR}/scene.cc
    ${SPCTRNTS_MAIN_DIR}/material.cc
    ${SPCTRNTS_MAIN_DIR}/texture.cc
    ${SPCTRNTS_MAIN_DIR}/mesh.cc
    ${SPCTRNTS_MAIN_DIR}/light.cc
    ${SPCTRNTS_MAIN_DIR}/camera.cc
    ${SPCTRNTS_MAIN_DIR}/node.cc
    ${SPCTRNTS_MAIN_DIR}/assetlibrary.cc
    ${SPCTRNTS_MAIN_DIR}/bvh.cc
    ${SPCTRNTS_MAIN_DIR}/animation.cc
    ${SPCTRNTS_MAIN_DIR}/keyframesampler.cc
    ${SPCTRNTS_MAIN_DIR}/skin.cc
    ${SPCTRNTS_MAIN_DIR}/tracablestructure.cc
    ${SPCTRNTS_MAIN_DIR}/sceneloader.cc
    ${SPCTRNTS_MAIN_DIR}/spectrum.cc
)
list(APPEND SPCTRNTS_CORE_SRCS
    ${SPCTRNTS_MAIN_DIR}/types.h
    ${SPCTRNTS_MAIN_DIR}/ray.h
    ${SPCTRNTS_MAIN_DIR}/aabb.h
    ${SPCTRNTS_MAIN_DIR}/config.h
    ${SPCTRNTS_MAIN_DIR}/framebuffer.h
    ${SPCTRNTS_MAIN_DIR}/postprocessor.h
    ${SPCTRNTS_MAIN_DIR}/renderer.h
    ${SPCTRNTS_MAIN_DIR}/renderscheduler.h
    ${SPCTRNTS_MAIN_DIR}/scene.h
    ${SPCTRNTS_MAIN_DIR}/material.h
    ${SPCTRNTS_MAIN_DIR}/texture.h
    ${SPCTRNTS_MAIN_DIR}/mesh.h
    ${SPCTRNTS_MAIN_DIR}/light.h
    ${SPCTRNTS_MAIN_DIR}/camera.h
    ${SPCTRNTS_MAIN_DIR}/node.h
    ${SPCTRNTS_MAIN_DIR}/assetlibrary.h
    ${SPCTRNTS_MAIN_DIR}/bvh.h
    ${SPCTRNTS_MAIN_DIR}/random.h
    ${SPCTRNTS_MAIN_DIR}/animation.h
    ${SPCTRNTS_MAIN_DIR}/keyframesampler.h
    ${SPCTRNTS_MAIN_DIR}/skin.h
    ${SPCTRNTS_MAIN_DIR}/tracablestructure.h
    ${SPCTRNTS_MAIN_DIR}/intersection.h
    ${SPCTRNTS_MAIN_DIR}/sceneloader.h
    ${SPCTRNTS_MAIN_DIR}/spectrum.h
)

add_library(SpectrenotesCore ${SPCTRNTS_CORE_SRCS})
target_include_directories(SpectrenotesCore
    PRIVATE
    ${SPCTRNTS_SOURCE_DIR}
    ${SPCTRNTS_CORE_DIR}
    ${SPCTRNTS_THIRDPARTY_DIR}
)

# executable settingsset (PLATFORM_DEPEND_LIBS )
set (PLATFORM_DEPEND_LIBS )
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
set (PLATFORM_DEPEND_LIBS winmm.lib)
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

add_executable(Spectrenotes
    ${SPCTRNTS_MAIN_DIR}/main.cc
)
target_link_libraries(Spectrenotes PUBLIC
    SpectrenotesCore
    LinearAlgebra
    ThirdpartyLibs
    ${PLATFORM_DEPEND_LIBS}
)
target_include_directories(Spectrenotes PRIVATE
    ${SPCTRNTS_SOURCE_DIR}
    ${SPCTRNTS_THIRDPARTY_DIR}
    ${SPCTRNTS_MAIN_DIR}
)
