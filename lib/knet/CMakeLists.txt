cmake_minimum_required(VERSION 3.5)

project(free5GC_knet C)

link_directories("${LIBGTP5GNL_DST}/lib" ${LOGGER_DST})

# Sources
file(GLOB SRC_FILES "src/*.c")
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} free5GC_utlt mnl logger)
target_include_directories(${PROJECT_NAME} PRIVATE
    include
    ${LOGGER_DST}
    "${CMAKE_SOURCE_DIR}/lib/utlt/include"
)
#target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)

# Test
add_executable(${PROJECT_NAME}_test test.c)
set_target_properties(${PROJECT_NAME}_test PROPERTIES 
    OUTPUT_NAME "${BUILD_BIN_DIR}/testknet"
)

target_link_libraries(${PROJECT_NAME}_test free5GC_utlt logger ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}_test PRIVATE
    include
    ${LOGGER_DST}
    "${CMAKE_SOURCE_DIR}/lib/utlt/include"
)
#target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
