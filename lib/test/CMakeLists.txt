cmake_minimum_required(VERSION 3.5)

project(free5GC_utlt_test C)

set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/lib/test/assets")
set(BUILD_ASSETS_DIR "${BUILD_BIN_DIR}/testutlt_assets")

file(MAKE_DIRECTORY ${BUILD_ASSETS_DIR})
file(GLOB ASSET_FILES "${ASSETS_DIR}/**")
file(COPY ${ASSET_FILES} DESTINATION ${BUILD_ASSETS_DIR})

link_directories(${LOGGER_DST})

# Test cases
file(GLOB SRC_FILES "src/*.c")
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} free5GC_utlt logger yaml)
target_include_directories(${PROJECT_NAME} PRIVATE
    include
    ${LOGGER_DST}
    "${CMAKE_SOURCE_DIR}/lib/utlt/include"
)

# Test main
add_executable(testutlt "test.c")
set_target_properties(testutlt PROPERTIES 
    OUTPUT_NAME "${BUILD_BIN_DIR}/testutlt"
)

target_link_libraries(testutlt free5GC_utlt logger yaml ${PROJECT_NAME})
target_include_directories(testutlt PRIVATE
    include
    ${LOGGER_DST}
    "${CMAKE_SOURCE_DIR}/lib/utlt/include"
)
#target_compile_options(${PROJECT_NAME} PRIVATE -g -ggdb -Wall -Werror)
