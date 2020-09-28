cmake_minimum_required(VERSION 3.5)

project(free5GC_updk C)

set(UPDK_SOURCE_DIR "${CMAKE_SOURCE_DIR}/updk")

# Sources
file(GLOB SRC_FILES "src/*.c")
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    include
)
#target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)

# Submodules
if("${UPDK_PKTPROC_MODULE}" STREQUAL "kernel")
    add_subdirectory(src/kernel)
# elseif("${UPDK_PKTPROC_MODULE}" STREQUAL "dpdk")
    # add_subdirectory(src/dpdk)
endif()