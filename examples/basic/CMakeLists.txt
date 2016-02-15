set(EXEC_NAME "basic")

# Add source to the executable
add_executable(
    ${EXEC_NAME}
    basic.c
)

# Link the executable to the library
target_link_libraries(${EXEC_NAME} viface)

# Set dependency with the library target
add_dependencies(${PROJECT_NAME} ${EXEC_NAME})
