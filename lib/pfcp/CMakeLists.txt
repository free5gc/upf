cmake_minimum_required(VERSION 3.5)

project(free5GC_pfcp C)

set(PFCP_SUPPORT_HDR "${CMAKE_SOURCE_DIR}/lib/pfcp/include/pfcp_message.h")
set(PFCP_SUPPORT_SRC "${CMAKE_SOURCE_DIR}/lib/pfcp/src/pfcp_message.c")

# Generate header files from spec
#add_custom_command(OUTPUT ${PFCP_SUPPORT_HDR}
#    COMMENT "Generating header files from spec"
#    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/support"

    # check if pip3 and python3 is installed
#    COMMAND which pip3 && which python3
#    COMMAND pip3 install -r requirements.txt
#    COMMAND python3 pfcpDocxParser.py
#    COMMAND cp pfcp_message.h ${PFCP_SUPPORT_HDR}
#    COMMAND cp pfcp_message.c ${PFCP_SUPPORT_SRC}
#)
#add_custom_target(pfcp_support ALL
#    DEPENDS ${PFCP_SUPPORT_HDR}
#    VERBATIM
#)

link_directories(${LOGGER_DST})

# Sources
file(GLOB SRC_FILES "src/*.c")
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} free5GC_utlt logger)
target_include_directories(${PROJECT_NAME} PRIVATE
    include
    ${LOGGER_DST}
    "${CMAKE_SOURCE_DIR}/lib/utlt/include"
)
#target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
#add_dependencies(${PROJECT_NAME} pfcp_support)
