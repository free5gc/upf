#!/usr/bin/env bash

# UPF
UPF_PATH=`pwd`
export GO111MODULE="off"
# Fix CMakeList
perl -i -pe 'BEGIN{undef $/;} s/"\/gofree5gc/"\/free5gc/g' `find ./ -name '*.h'`
# Modify cmake
echo 'set(CMAKE_C_COMPILER "/usr/bin/gcc")
cmake_minimum_required(VERSION 3.5)

project(free5GC_UPF C)

# Build destination
set(BUILD_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set(BUILD_CONFIG_DIR "${CMAKE_BINARY_DIR}/config")

# Build paths
set(LIBGTPNL_SRC "${CMAKE_SOURCE_DIR}/lib/libgtpnl-1.2.1")
set(LIBGTPNL_DST "${CMAKE_BINARY_DIR}/libgtpnl")
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
)
add_custom_target(configs ALL DEPENDS ${BUILD_CONFIG_DIR} VERBATIM)

add_compile_options(-Wall -Werror -Wno-address-of-packed-member)

# Submodules
add_subdirectory(src)
add_subdirectory(lib)
add_subdirectory(lib/gtpv1)
add_subdirectory(lib/utlt)' > ${UPF_PATH}/CMakeLists.txt
echo 'cmake_minimum_required(VERSION 3.5)

project(free5GC_gtpv1 C)

set(LIBGTPNL_DST_SO "${LIBGTPNL_DST}/lib/libgtpnl.so")

# Build libgtpnl
# Add_custom_command does not create a new target. You have to define targets explicitly
# by add_executable, add_library or add_custom_target in order to make them visible to make.
add_custom_command(OUTPUT ${LIBGTPNL_DST_SO}
    # Display the given message before the commands are executed at build time
    COMMENT "Building libgtpnl"
    WORKING_DIRECTORY ${LIBGTPNL_SRC}

    COMMAND chmod +x git-version-gen
    COMMAND autoreconf -iv
    COMMAND ./configure --prefix=${LIBGTPNL_DST}
    COMMAND make -j`nproc`
    COMMAND make install
)
add_custom_target(libgtpnl ALL
    # This is ALL target "libgtpnl", and it depends on ${LIBGTPNL_MAKEFILE}"
    # If the file exists, then commands related to that file wont be executed.
    # DONOT let other target depends on the same OUTPUT as current target,
    #   or it may be bad when doing parallel make.
    DEPENDS ${LIBGTPNL_DST_SO}

    # To make quotes printable
    VERBATIM
)

link_directories("${LIBGTPNL_DST}/lib" ${LOGGER_DST})

# Test
add_executable("${PROJECT_NAME}_test" test.c)
set_target_properties("${PROJECT_NAME}_test" PROPERTIES
    OUTPUT_NAME "${BUILD_BIN_DIR}/testgtpv1"
)

target_link_libraries("${PROJECT_NAME}_test" free5GC_lib gtpnl logger)
target_include_directories("${PROJECT_NAME}_test" PRIVATE
    include
    ${LOGGER_DST}
    "${LIBGTPNL_DST}/include"
    "${CMAKE_SOURCE_DIR}/lib/utlt/include"
)
target_compile_options("${PROJECT_NAME}_test" PRIVATE -Wall -Werror)
add_dependencies("${PROJECT_NAME}_test" utlt_logger libgtpnl)' > ${UPF_PATH}/lib/gtpv1/CMakeLists.txt
echo 'cmake_minimum_required(VERSION 3.5)

project(free5GC_utlt_logger C)

link_directories(${LOGGER_DST})

# Logger
add_custom_command(OUTPUT ${LOGGER_DST}
    COMMENT "Building utlt_logger"

    WORKING_DIRECTORY ${LOGGER_SRC}
    COMMAND go build -o ${LOGGER_DST}/liblogger.so -buildmode=c-shared
    COMMAND mv ${LOGGER_DST}/liblogger.h ${LOGGER_DST}/logger.h
)
add_custom_target(utlt_logger ALL
    DEPENDS ${LOGGER_DST}
    VERBATIM
)
link_directories(${LOGGER_DST})' > ${UPF_PATH}/lib/utlt/CMakeLists.txt
echo 'cmake_minimum_required(VERSION 3.5)

project(free5GC_UPF_main C)

link_directories("${LIBGTPNL_DST}/lib" ${LOGGER_DST})

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
    "${LIBGTPNL_DST}/include"
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/lib/gtpv1/include"
    "${CMAKE_SOURCE_DIR}/lib/knet/include"
    "${CMAKE_SOURCE_DIR}/lib/pfcp/include"
    "${CMAKE_SOURCE_DIR}/lib/utlt/include"
    "${CMAKE_SOURCE_DIR}/lib/utlt/logger/include"
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    free5GC_lib gtpnl logger yaml
)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
add_dependencies(${PROJECT_NAME} utlt_logger libgtpnl)' > ${UPF_PATH}/src/CMakeLists.txt
echo 'cmake_minimum_required(VERSION 3.5)

project(free5GC_lib)

set(LIBGTPNL_DST_SO "${LIBGTPNL_DST}/lib/libgtpnl.so")

link_directories(${LOGGER_DST})

# Sources
file(GLOB SRC_FILES
    "gtpv1/src/*.c"
    "knet/src/*.c"
    "pfcp/src/*.c"
    "utlt/src/*.c"
)
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} rt pthread mnl logger)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${LOGGER_DST}
    "${LIBGTPNL_DST}/include"
    "${CMAKE_SOURCE_DIR}/lib/gtpv1/include"
    "${CMAKE_SOURCE_DIR}/lib/knet/include"
    "${CMAKE_SOURCE_DIR}/lib/pfcp/include"
    "${CMAKE_SOURCE_DIR}/lib/utlt/include"
)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
add_dependencies(${PROJECT_NAME} utlt_logger libgtpnl)' > ${UPF_PATH}/lib/CMakeLists.txt

rm -f ${UPF_PATH}/lib/pfcp/CMakeLists.txt
rm -f ${UPF_PATH}/test/CMakeLists.txt
# Build UPF
cd ${UPF_PATH} && mkdir -p build && cd build
cmake .. && make -j `nproc`
cp lib/libfree5GC_lib.a ${UPF_PATH}/lib
# Remove unrelease file
cd ${UPF_PATH}
rm -rf lib/test/
rm -rf lib/pfcp/src lib/pfcp/support lib/pfcp/test
rm -rf lib/knet/src lib/knet/CMakeLists.txt lib/knet/test.c
rm -f lib/CMakeLists.txt
rm -rf lib/gtpv1/src
rm -rf lib/utlt/src
cd ${ROOT_PATH}
# Remove build tmp file
echo 'set(CMAKE_C_COMPILER "/usr/bin/gcc")
cmake_minimum_required(VERSION 3.5)

project(free5GC_UPF C)

# Build destination
set(BUILD_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set(BUILD_CONFIG_DIR "${CMAKE_BINARY_DIR}/config")

# Build paths
set(LIBGTPNL_SRC "${CMAKE_SOURCE_DIR}/lib/libgtpnl-1.2.1")
set(LIBGTPNL_DST "${CMAKE_BINARY_DIR}/libgtpnl")
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
)
add_custom_target(configs ALL DEPENDS ${BUILD_CONFIG_DIR} VERBATIM)

# Submodules
add_subdirectory(src)
add_subdirectory(lib/gtpv1)
add_subdirectory(lib/utlt)' > ${UPF_PATH}/CMakeLists.txt
echo 'cmake_minimum_required(VERSION 3.5)

project(free5GC_gtpv1 C)

set(LIBGTPNL_DST_SO "${LIBGTPNL_DST}/lib/libgtpnl.so")

# Build libgtpnl
# Add_custom_command does not create a new target. You have to define targets explicitly
# by add_executable, add_library or add_custom_target in order to make them visible to make.
add_custom_command(OUTPUT ${LIBGTPNL_DST_SO}
    # Display the given message before the commands are executed at build time
    COMMENT "Building libgtpnl"
    WORKING_DIRECTORY ${LIBGTPNL_SRC}

    COMMAND chmod +x git-version-gen
    COMMAND autoreconf -iv
    COMMAND ./configure --prefix=${LIBGTPNL_DST}
    COMMAND make -j`nproc`
    COMMAND make install
)
add_custom_target(libgtpnl ALL
    # This is ALL target "libgtpnl", and it depends on ${LIBGTPNL_MAKEFILE}"
    # If the file exists, then commands related to that file wont be executed.
    # DONOT let other target depends on the same OUTPUT as current target,
    #   or it may be bad when doing parallel make.
    DEPENDS ${LIBGTPNL_DST_SO}

    # To make quotes printable
    VERBATIM
)

link_directories("${CMAKE_SOURCE_DIR}/lib" "${LIBGTPNL_DST}/lib" ${LOGGER_DST})

# Test
add_executable("${PROJECT_NAME}_test" test.c)
set_target_properties("${PROJECT_NAME}_test" PROPERTIES
    OUTPUT_NAME "${BUILD_BIN_DIR}/testgtpv1"
)

target_link_libraries("${PROJECT_NAME}_test" free5GC_lib gtpnl logger)
target_include_directories("${PROJECT_NAME}_test" PRIVATE
    include
    ${LOGGER_DST}
    "${LIBGTPNL_DST}/include"
    "${CMAKE_SOURCE_DIR}/lib/utlt/include"
)
target_compile_options("${PROJECT_NAME}_test" PRIVATE -Wall -Werror)
add_dependencies("${PROJECT_NAME}_test" utlt_logger libgtpnl)' > ${UPF_PATH}/lib/gtpv1/CMakeLists.txt
echo 'cmake_minimum_required(VERSION 3.5)

project(free5GC_UPF_main C)

link_directories("${CMAKE_SOURCE_DIR}/lib" "${LIBGTPNL_DST}/lib" ${LOGGER_DST})

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
    "${LIBGTPNL_DST}/include"
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/lib/gtpv1/include"
    "${CMAKE_SOURCE_DIR}/lib/knet/include"
    "${CMAKE_SOURCE_DIR}/lib/pfcp/include"
    "${CMAKE_SOURCE_DIR}/lib/utlt/include"
    "${CMAKE_SOURCE_DIR}/lib/utlt/logger/include"
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    free5GC_lib rt pthread gtpnl mnl logger yaml
)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
add_dependencies(${PROJECT_NAME} utlt_logger libgtpnl)' > ${UPF_PATH}/src/CMakeLists.txt

go get -u github.com/sirupsen/logrus

cd ${UPF_PATH}/build
make clean && cmake .. && make -j `nproc`

rm -rf ${UPF_PATH}/build
rm -f ${UPF_PATH}/upf_release.sh

