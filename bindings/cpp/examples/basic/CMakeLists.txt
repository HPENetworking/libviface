set(EXEC_NAME "basiccpp")

# Add source to the executable
add_executable(
    ${EXEC_NAME}
    basic.cpp
)

# Link the executable to the library
target_link_libraries(${EXEC_NAME} vifacecpp)

# Set dependency with the library target
add_dependencies(${PROJECT_NAME} ${EXEC_NAME})
