cmake_minimum_required(VERSION 3.5)

project(free5GC_utlt C)

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

# Sources
file(GLOB SRC_FILES "src/*.c")
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} rt pthread logger yaml)
target_include_directories(${PROJECT_NAME} PRIVATE include ${LOGGER_DST})
#target_compile_options(${PROJECT_NAME} PRIVATE -g -ggdb -Wall -Werror)
add_dependencies(${PROJECT_NAME} utlt_logger)
