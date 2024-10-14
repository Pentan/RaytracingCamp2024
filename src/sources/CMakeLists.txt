# directories
set(PETALS_LINEARALGEBRA_DIR linearalgebra)
set(PETALS_MAIN_DIR petals)

# LinearAlgebra
set(LINEARALGEBRA_SRCS
    ${PETALS_LINEARALGEBRA_DIR}/linearalgebra.h
    ${PETALS_LINEARALGEBRA_DIR}/constants.h
    ${PETALS_LINEARALGEBRA_DIR}/vector3.h
    ${PETALS_LINEARALGEBRA_DIR}/vector4.h
    ${PETALS_LINEARALGEBRA_DIR}/matrix4.h
    ${PETALS_LINEARALGEBRA_DIR}/quaternion.h
    ${PETALS_LINEARALGEBRA_DIR}/vector3.inl
    ${PETALS_LINEARALGEBRA_DIR}/vector4.inl
    ${PETALS_LINEARALGEBRA_DIR}/matrix4.inl
    ${PETALS_LINEARALGEBRA_DIR}/quaternion.inl
)
add_library(LinearAlgebra INTERFACE)
add_custom_target(LinearAlgebra_h SOURCES ${LINEARALGEBRA_SRCS})

# Core
list(APPEND PETALS_CORE_SRCS
    ${PETALS_MAIN_DIR}/aabb.cc
    ${PETALS_MAIN_DIR}/config.cc
    ${PETALS_MAIN_DIR}/framebuffer.cc
    ${PETALS_MAIN_DIR}/postprocessor.cc
    ${PETALS_MAIN_DIR}/renderer.cc
    ${PETALS_MAIN_DIR}/renderscheduler.cc
    ${PETALS_MAIN_DIR}/scene.cc
    ${PETALS_MAIN_DIR}/material.cc
    ${PETALS_MAIN_DIR}/texture.cc
    ${PETALS_MAIN_DIR}/mesh.cc
    ${PETALS_MAIN_DIR}/light.cc
    ${PETALS_MAIN_DIR}/camera.cc
    ${PETALS_MAIN_DIR}/node.cc
    ${PETALS_MAIN_DIR}/assetlibrary.cc
    ${PETALS_MAIN_DIR}/bvh.cc
    ${PETALS_MAIN_DIR}/animation.cc
    ${PETALS_MAIN_DIR}/keyframesampler.cc
    ${PETALS_MAIN_DIR}/skin.cc
    ${PETALS_MAIN_DIR}/tracablestructure.cc
    ${PETALS_MAIN_DIR}/sceneloader_gltf.cc
    ${PETALS_MAIN_DIR}/sceneloader_animstand.cc
    ${PETALS_MAIN_DIR}/spectrum.cc
    ${PETALS_MAIN_DIR}/animstand.cc
)
list(APPEND PETALS_CORE_SRCS
    ${PETALS_MAIN_DIR}/types.h
    ${PETALS_MAIN_DIR}/ray.h
    ${PETALS_MAIN_DIR}/aabb.h
    ${PETALS_MAIN_DIR}/config.h
    ${PETALS_MAIN_DIR}/framebuffer.h
    ${PETALS_MAIN_DIR}/postprocessor.h
    ${PETALS_MAIN_DIR}/renderer.h
    ${PETALS_MAIN_DIR}/renderscheduler.h
    ${PETALS_MAIN_DIR}/scene.h
    ${PETALS_MAIN_DIR}/material.h
    ${PETALS_MAIN_DIR}/texture.h
    ${PETALS_MAIN_DIR}/mesh.h
    ${PETALS_MAIN_DIR}/light.h
    ${PETALS_MAIN_DIR}/camera.h
    ${PETALS_MAIN_DIR}/node.h
    ${PETALS_MAIN_DIR}/assetlibrary.h
    ${PETALS_MAIN_DIR}/bvh.h
    ${PETALS_MAIN_DIR}/random.h
    ${PETALS_MAIN_DIR}/animation.h
    ${PETALS_MAIN_DIR}/keyframesampler.h
    ${PETALS_MAIN_DIR}/skin.h
    ${PETALS_MAIN_DIR}/tracablestructure.h
    ${PETALS_MAIN_DIR}/intersection.h
    ${PETALS_MAIN_DIR}/sceneloader.h
    ${PETALS_MAIN_DIR}/spectrum.h
    ${PETALS_MAIN_DIR}/animstand.h
)

add_library(PetalsCore ${PETALS_CORE_SRCS})
target_include_directories(PetalsCore
    PRIVATE
    ${PETALS_SOURCE_DIR}
    ${PETALS_CORE_DIR}
    ${PETALS_THIRDPARTY_DIR}
)

# executable settingsset (PLATFORM_DEPEND_LIBS )
set (PLATFORM_DEPEND_LIBS )
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
set (PLATFORM_DEPEND_LIBS winmm.lib)
endif(CMAKE_SYSTEM_NAME STREQUAL "Windows")

add_executable(Petals
    ${PETALS_MAIN_DIR}/main.cc
)
target_link_libraries(Petals PUBLIC
    PetalsCore
    LinearAlgebra
    ThirdpartyLibs
    ${PLATFORM_DEPEND_LIBS}
)
target_include_directories(Petals PRIVATE
    ${PETALS_SOURCE_DIR}
    ${PETALS_THIRDPARTY_DIR}
    ${PETALS_MAIN_DIR}
)
