# Define target name
set(WRAPPER_NAME "vifacego")

# Creates custom target to build library
add_custom_target(
	${WRAPPER_NAME}
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Building ${WRAPPER_NAME} GO wrapper..." VERBATIM
)

# Creates custom command to install libviface GO wrapper
add_custom_command(
    TARGET ${WRAPPER_NAME}
    COMMAND go install vifacego    
    COMMENT "Building libviface GO packet..."
)

# Set dependency with the library target
add_dependencies(${PROJECT_NAME} ${WRAPPER_NAME})
