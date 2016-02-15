cmake_minimum_required(VERSION 2.8)

# Define project name
project("libvifacego")

# The version number
set(libvifacego_VERSION_STRING "1.1.0")

# Creates custom target to build library
add_custom_target(
	${PROJECT_NAME}
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Building ${PROJECT_NAME} library..." VERBATIM
)

add_subdirectory("${libvifacego_SOURCE_DIR}/src/")

# Moves bin GO files to cmake build directory
add_custom_command(
    TARGET ${PROJECT_NAME}
    COMMAND rm -rf ./bin
    COMMAND rm -rf ../../../bindings/go/pkg
    COMMAND mv ../../../bindings/go/bin ./
)


# Creates custom target to build library
add_custom_target(
	docgo
	COMMAND  godoc -http=:6060 -goroot=../../../build/bindings/go/
        COMMENT "${PROJECT_NAME} GO documentation available at http://localhost:6060/pkg/vifacego/"
)










