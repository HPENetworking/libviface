find_package(Threads)

if(Threads_FOUND)
    set(EXEC_NAME "threadscpp")

    # Add source to the executable
    add_executable(
        ${EXEC_NAME}
        threads.cpp
    )

    # Link the executable to the library
    target_link_libraries(${EXEC_NAME} vifacecpp ${CMAKE_THREAD_LIBS_INIT})
else()
    message(WARNING "Threads package not found, not building threads example...")
endif()

# Set dependency with the library target
add_dependencies(${PROJECT_NAME} ${EXEC_NAME})
