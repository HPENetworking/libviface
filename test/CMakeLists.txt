set(EXEC_NAME "tests")

# Add source to the executable
add_executable(
    ${EXEC_NAME}
    create.cpp
)

# Link the executable to the library
target_link_libraries(${EXEC_NAME} viface)
