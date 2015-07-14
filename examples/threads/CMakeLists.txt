find_package(Threads)

set(EXEC_NAME "threads")

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
    ${EXEC_NAME}.cpp
)

# Link the executable to the library
target_link_libraries(${EXEC_NAME} viface ${CMAKE_THREAD_LIBS_INIT})
