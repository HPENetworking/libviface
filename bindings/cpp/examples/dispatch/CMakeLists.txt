set(EXEC_NAME "dispatchcpp")

# Add source to the executable
add_executable(
    ${EXEC_NAME}
    dispatch.cpp
)

# Link the executable to the library
target_link_libraries(${EXEC_NAME} vifacecpp)

# Set dependency with the library target
add_dependencies(${PROJECT_NAME} ${EXEC_NAME})
