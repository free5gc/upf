cmake_minimum_required(VERSION 3.5)

project(free5GC_UPF_main C)

link_directories(${LOGGER_DST})

# Sources
file(GLOB SRC_FILES
    "*.c"
    "n4/*.c"
    "up/*.c"
)
add_executable(${PROJECT_NAME} ${SRC_FILES})
set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        OUTPUT_NAME "${BUILD_BIN_DIR}/free5gc-upfd"
        SUFFIX ""
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${LOGGER_DST}
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/lib/pfcp/include"
    "${CMAKE_SOURCE_DIR}/lib/utlt/include"
    "${CMAKE_SOURCE_DIR}/lib/utlt/logger/include"
    "${CMAKE_SOURCE_DIR}/updk/include"
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    # TODO: "free5GC_updk_kernel" is hard code for using updk/src/kernel, it may be changed if use other repo
    free5GC_updk free5GC_updk_${UPDK_PKTPROC_MODULE} free5GC_utlt free5GC_pfcp logger yaml
)
#target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
