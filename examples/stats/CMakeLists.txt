set(EXEC_NAME "stats")

# Add source to the executable
add_executable(
    ${EXEC_NAME}
    ${EXEC_NAME}.c
)

# Link the executable to the library
target_link_libraries(${EXEC_NAME} viface)

# Set dependency with the library target
add_dependencies(${PROJECT_NAME} ${EXEC_NAME})
