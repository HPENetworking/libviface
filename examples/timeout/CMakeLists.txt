set(EXEC_NAME "timeout")

# Allow to use libviface
include_directories(
    "${libviface_SOURCE_DIR}/include"
)

# Add source to the executable
add_executable(
    ${EXEC_NAME}
    ${EXEC_NAME}.cpp
)

# Link the executable to the library
target_link_libraries(${EXEC_NAME} viface)