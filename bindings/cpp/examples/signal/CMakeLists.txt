set(EXEC_NAME "signalcpp")

# Add source to the executable
add_executable(
    ${EXEC_NAME}
    signal.cpp
)

# Link the executable to the library
target_link_libraries(${EXEC_NAME} vifacecpp)

# Set dependency with the library target
add_dependencies(${PROJECT_NAME} ${EXEC_NAME})
