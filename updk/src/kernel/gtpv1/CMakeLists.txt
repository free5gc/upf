cmake_minimum_required(VERSION 3.5)

project(free5GC_gtpv1 C)

# Build libgtp5gnl
# Add_custom_command does not create a new target. You have to define targets explicitly
# by add_executable, add_library or add_custom_target in order to make them visible to make.
add_custom_command(OUTPUT ${LIBGTP5GNL_DST_SO}
    # Display the given message before the commands are executed at build time
    COMMENT "Building libgtp5gnl"
    WORKING_DIRECTORY ${LIBGTP5GNL_SRC}
    
    COMMAND chmod +x git-version-gen
    COMMAND autoreconf -iv
    COMMAND ./configure --prefix=${LIBGTP5GNL_DST}
    COMMAND make -j`nproc`
    COMMAND make install
)
add_custom_target(libgtp5gnl ALL
    # This is ALL target 'libgtp5gnl', and it depends on ${LIBGTP5GNL_MAKEFILE}"
    # If the file exists, then commands related to that file won't be executed.
    # DONOT let other target depends on the same OUTPUT as current target,
    #   or it may be bad when doing parallel make.
    DEPENDS ${LIBGTP5GNL_DST_SO}

    # To make quotes printable
    VERBATIM
)

link_directories("${LIBGTP5GNL_DST}/lib" ${LOGGER_DST})

# Sources
file(GLOB SRC_FILES "src/*.c")
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} free5GC_utlt ${LIBGTP5GNL_DST_SO} logger)
target_include_directories(${PROJECT_NAME} PRIVATE
    include
    ${LOGGER_DST}
    "${LIBGTP5GNL_DST}/include"
    "${CMAKE_SOURCE_DIR}/lib/utlt/include"
)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
add_dependencies(${PROJECT_NAME} libgtp5gnl)