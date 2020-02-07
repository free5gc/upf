set(CMAKE_C_COMPILER "/usr/bin/gcc")
cmake_minimum_required(VERSION 3.5)

project(free5GC_UPF C)

# User build options
# UPDK_PKTPROC_MODULE: packet processing module used by UPDK
#  - either: "kernel" | ...
set(UPDK_PKTPROC_MODULE "kernel")

# Build destination
set(BUILD_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set(BUILD_CONFIG_DIR "${CMAKE_BINARY_DIR}/config")

# Build paths
set(LOGGER_SRC "${CMAKE_SOURCE_DIR}/lib/utlt/logger")
set(LOGGER_DST "${CMAKE_BINARY_DIR}/utlt_logger")

# Build environment
file(MAKE_DIRECTORY ${BUILD_BIN_DIR})

# Config files
set(CONFIG_SRC "${CMAKE_SOURCE_DIR}/config")
file(GLOB CONFIG_FILES "${CONFIG_SRC}/**")
add_custom_command(OUTPUT ${BUILD_CONFIG_DIR}
    COMMENT "Copying configs"
    WORKING_DIRECTORY ${CONFIG_SRC}

    COMMAND mkdir -p ${BUILD_CONFIG_DIR} >/dev/null 2>&1
    COMMAND cp "${CONFIG_SRC}/upfcfg.example.yaml" "${BUILD_CONFIG_DIR}/upfcfg.yaml"
    COMMAND cp "${CONFIG_SRC}/upfcfg.test.example.yaml" "${BUILD_CONFIG_DIR}/upfcfg.test.yaml"
    COMMAND cp "${CONFIG_SRC}/upfcfg.ulcl.example.yaml" "${BUILD_CONFIG_DIR}/upfcfg.ulcl.yaml"
)
add_custom_target(configs ALL DEPENDS ${BUILD_CONFIG_DIR} VERBATIM)

add_compile_options(-Wall -Werror -Wno-address-of-packed-member)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g3 -O0")

# Submodules
add_subdirectory(src)
add_subdirectory(updk)
add_subdirectory(lib/pfcp)
add_subdirectory(lib/pfcp/test)
add_subdirectory(lib/utlt)
add_subdirectory(lib/test)
