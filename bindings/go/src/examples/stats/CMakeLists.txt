# Define target name
set(EXEC_NAME "statsgo")

# Creates custom target to build stats GO example
add_custom_target(
	${EXEC_NAME}
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Creates custom command to install stats GO example
add_custom_command(
    TARGET ${EXEC_NAME}
    COMMAND go install examples/stats
    COMMENT "Building ${EXEC_NAME} GO example..."
)

# Set dependency with the library target
add_dependencies(${PROJECT_NAME} ${EXEC_NAME})
