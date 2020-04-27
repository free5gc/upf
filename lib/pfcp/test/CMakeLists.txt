cmake_minimum_required(VERSION 3.5)

project(free5GC_pfcp_test)

set(TESTBIN_DIR "${BUILD_BIN_DIR}/testbin_pfcp")
file(MAKE_DIRECTORY ${TESTBIN_DIR})
file(GLOB SRC_FILES "src/*.c")

link_directories(${LOGGER_DST})

foreach(file_path ${SRC_FILES})
    string(REPLACE ".c" "" new_name ${file_path})
    get_filename_component(filename ${new_name} NAME)
    set(target_name "pfcp_${filename}")

    add_executable(${target_name} ${file_path})
    set_target_properties(${target_name} PROPERTIES 
        OUTPUT_NAME "${TESTBIN_DIR}/${filename}"
    )

    target_link_libraries(${target_name} free5GC_utlt free5GC_pfcp logger)
    target_include_directories(${target_name} PRIVATE
        include
        ${LOGGER_DST}
        "${CMAKE_SOURCE_DIR}/lib/pfcp/include"
        "${CMAKE_SOURCE_DIR}/lib/utlt/include"
    )
endforeach()


add_executable(testpfcp "test.c")
set_target_properties(testpfcp PROPERTIES 
    OUTPUT_NAME "${BUILD_BIN_DIR}/testpfcp"
)
