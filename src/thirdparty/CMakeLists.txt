# thirdparty settings
set(THIRDPARTY_SRCS
    ${PETALS_THIRDPARTY_DIR}/thirdparty.cc
)
set(THIRDPARTY_STB_SRCS
    ${PETALS_THIRDPARTY_DIR}/stb/stb_image.h
    ${PETALS_THIRDPARTY_DIR}/stb/stb_image_write.h
)
set(THIRDPARTY_JSON_SRCS
    ${PETALS_THIRDPARTY_DIR}/nlohmann/json.hpp
)
set(THIRDPARTY_TINYGLTF_SRCS
    ${PETALS_THIRDPARTY_DIR}/tinygltf/tiny_gltf.h
)

add_library(ThirdpartyLibs
    ${THIRDPARTY_SRCS}
    ${THIRDPARTY_STB_SRCS}
    ${THIRDPARTY_JSON_SRCS}
    ${THIRDPARTY_TINYGLTF_SRCS}
)
target_include_directories(ThirdpartyLibs
    PUBLIC
    ${PETALS_THIRDPARTY_DIR}
)

source_group(stb FILES ${THIRDPARTY_STB_SRCS})
source_group(nlohmann FILES ${THIRDPARTY_JSON_SRCS})
source_group(tinygltf FILES ${THIRDPARTY_TINYGLTF_SRCS})
