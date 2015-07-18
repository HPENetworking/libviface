find_package(Threads)

if(Threads_FOUND)
    set(EXEC_NAME "threads")

    # Add source to the executable
    add_executable(
        ${EXEC_NAME}
        ${EXEC_NAME}.cpp
    )

    # Link the executable to the library
    target_link_libraries(${EXEC_NAME} viface ${CMAKE_THREAD_LIBS_INIT})
else()
    message(WARNING "Threads package not found, not building threads example...")
endif()