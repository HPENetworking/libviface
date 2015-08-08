set(EXEC_NAME "vifaced")

# Add source to the executable
add_executable(
    ${EXEC_NAME}
    ${EXEC_NAME}.cpp
)

# Link the executable to the library
target_link_libraries(${EXEC_NAME} viface)