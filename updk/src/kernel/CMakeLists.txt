cmake_minimum_required(VERSION 3.5)

project(free5GC_updk_kernel C)

link_directories("${LIBGTP5GNL_DST}/lib" ${LOGGER_DST})

# Build destination
set(LIBGTP5GNL_SRC "${CMAKE_SOURCE_DIR}/updk/src/third_party/libgtp5gnl")
set(LIBGTP5GNL_DST "${CMAKE_BINARY_DIR}/updk/src/third_party/libgtp5gnl")
set(LIBGTP5GNL_DST_SO "${LIBGTP5GNL_DST}/lib/libgtp5gnl.so")

# Sources
file(GLOB_RECURSE SELF_FILES "*.c")
add_library(${PROJECT_NAME} STATIC ${SELF_FILES})

target_link_libraries(${PROJECT_NAME} free5GC_utlt free5GC_gtpv1 free5GC_knet free5GC_pfcp ${LIBGTP5GNL_DST_SO} logger)
target_include_directories(${PROJECT_NAME} PUBLIC
    "."
    ${LOGGER_DST}
    "${LIBGTP5GNL_DST}/include"
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/lib/pfcp/include"
    "${CMAKE_SOURCE_DIR}/lib/utlt/include"
    "${CMAKE_SOURCE_DIR}/lib/utlt/logger/include"
    "${UPDK_SOURCE_DIR}/include"
    "${UPDK_SOURCE_DIR}/src/kernel/knet/include"
    "${UPDK_SOURCE_DIR}/src/kernel/gtpv1/include"
)
#target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)

# Submodules
add_subdirectory(knet)
add_subdirectory(gtpv1)