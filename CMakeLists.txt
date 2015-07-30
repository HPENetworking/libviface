cmake_minimum_required(VERSION 2.8)

# Define project name
project("libviface")

# The version number
set(libviface_VERSION_STRING "1.1.0")

# Setup project variables
set(VIFACE_TESTS ON CACHE BOOL "Enable compilation of unit tests.")
set(VIFACE_EXAMPLES ON CACHE BOOL "Enable compilation of the examples.")
set(VIFACE_FORCE_32BITS OFF CACHE BOOL "Force library compilation for 32 bits.")

# Force C++11 compiler
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Installation variables
set(CMAKE_INSTALL_LIBDIR lib)
set(CMAKE_INSTALL_INCLUDEDIR include)
set(CMAKE_INSTALL_BINDIR bin)

# Configure library install location
if(VIFACE_FORCE_32BITS)
    set(CMAKE_INSTALL_LIBDIR lib32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32")
endif(VIFACE_FORCE_32BITS)

# Support for pkg-config file
set(pkgconfig_prefix ${CMAKE_INSTALL_PREFIX})
set(pkgconfig_exec_prefix ${CMAKE_INSTALL_PREFIX})
set(pkgconfig_libdir ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
set(pkgconfig_version ${libviface_VERSION_STRING})
configure_file(
    "${libviface_SOURCE_DIR}/libviface.pc.in"
    "${CMAKE_BINARY_DIR}/libviface.pc"
    @ONLY
)
install(
    FILES
    "${CMAKE_BINARY_DIR}/libviface.pc"
    DESTINATION
    "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)

# Configure library
configure_file(
    "${libviface_SOURCE_DIR}/include/viface/config.hpp.in"
    "${CMAKE_BINARY_DIR}/include/viface/config.hpp"
    @ONLY
)

# Specify include directory
include_directories("${libviface_SOURCE_DIR}/include/")
include_directories("${CMAKE_BINARY_DIR}/include/")

# Specify source directory
add_subdirectory("${libviface_SOURCE_DIR}/src/")
if(VIFACE_TESTS)
    message(STATUS "Including libviface test suite...")
    add_subdirectory("${libviface_SOURCE_DIR}/test/")
endif(VIFACE_TESTS)
if(VIFACE_EXAMPLES)
    message(STATUS "Including libviface examples...")
    add_subdirectory("${libviface_SOURCE_DIR}/examples/")
endif(VIFACE_EXAMPLES)

# Specify public interface install directory
install(
    FILES
    "${libviface_SOURCE_DIR}/include/viface/viface.hpp"
    "${CMAKE_BINARY_DIR}/include/viface/config.hpp"
    DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/viface"
)

# Add API Reference generation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(
        "${libviface_SOURCE_DIR}/doc/doxygen.conf.in"
        "${CMAKE_BINARY_DIR}/doxygen.conf"
        @ONLY
    )
    add_custom_target(
        doc
        "${DOXYGEN_EXECUTABLE}"
        "${CMAKE_BINARY_DIR}/doxygen.conf"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Generating API Reference documentation..." VERBATIM
    )
endif(DOXYGEN_FOUND)

# Setup Git hooks
if(EXISTS ${CMAKE_SOURCE_DIR}/.git/hooks)

    # Setup our git hooks to avoid people from checking in code against the
    # Coding Standard (at least syntactically)
    if(NOT EXISTS ${CMAKE_SOURCE_DIR}/.git/hooks/pre-commit)

        message(STATUS "Installing Coding Standard pre-commit hook...")
        execute_process(
            COMMAND ln -s ../../tools/uncrustify/pre-commit ${CMAKE_SOURCE_DIR}/.git/hooks/pre-commit
            COMMAND chmod 0755 ${CMAKE_SOURCE_DIR}/.git/hooks/pre-commit
        )
    endif()
endif()