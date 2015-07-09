set(EXEC_NAME "tests")

# Allow to use libviface
include_directories (
    ${libviface_SOURCE_DIR}/include
)
link_directories(
    ${libviface_SOURCE_DIR}/${LIB_NAME}
)

# Add source to the executable
add_executable(
    ${EXEC_NAME}
    create.cpp
)

# Link the executable to the library
target_link_libraries(${EXEC_NAME} viface)
