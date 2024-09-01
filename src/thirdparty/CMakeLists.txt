# thirdparty settings
set(THIRDPARTY_SRCS
    ${PINKYPI_THIRDPARTY_DIR}/thirdparty.cc
)
set(THIRDPARTY_STB_SRCS
    ${PINKYPI_THIRDPARTY_DIR}/stb/stb_image.h
    ${PINKYPI_THIRDPARTY_DIR}/stb/stb_image_write.h
)
set(THIRDPARTY_JSON_SRCS
    ${PINKYPI_THIRDPARTY_DIR}/nlohmann/json.hpp
)
set(THIRDPARTY_TINYGLTF_SRCS
    ${PINKYPI_THIRDPARTY_DIR}/tinygltf/tiny_gltf.h
)

add_library(ThirdpartyLibs
    ${THIRDPARTY_SRCS}
    ${THIRDPARTY_STB_SRCS}
    ${THIRDPARTY_JSON_SRCS}
    ${THIRDPARTY_TINYGLTF_SRCS}
)
target_include_directories(ThirdpartyLibs
    PUBLIC
    ${PINKYPI_THIRDPARTY_DIR}
)
# ${PINKYPI_THIRDPARTY_DIR}/stb
# ${PINKYPI_THIRDPARTY_DIR}/nlohmann
# ${PINKYPI_THIRDPARTY_DIR}/tinygltf

source_group(stb FILES ${THIRDPARTY_STB_SRCS})
source_group(nlohmann FILES ${THIRDPARTY_JSON_SRCS})
source_group(tinygltf FILES ${THIRDPARTY_TINYGLTF_SRCS})
