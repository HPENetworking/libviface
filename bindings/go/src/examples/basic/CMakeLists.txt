# Define target name
set(EXEC_NAME "basicgo")

# Creates custom target to build basic GO example
add_custom_target(
	${EXEC_NAME}
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Creates custom command to install basic GO example
add_custom_command(
    TARGET ${EXEC_NAME}
    COMMAND go install examples/basic
    COMMENT "Building ${EXEC_NAME} GO example..."
)

# Set dependency with the library target
add_dependencies(${PROJECT_NAME} ${EXEC_NAME})
