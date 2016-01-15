include(FindPkgConfig)

pkg_check_modules(LIBTINS "libtins")

if(LIBTINS_FOUND)
    set(EXEC_NAME "libtinscpp")

    # Allow to use libviface
    include_directories(
        "${LIBTINS_INCLUDE_DIRS}"
    )
    link_directories(
        "${LIBTINS_LIBRARY_DIRS}"
    )

    # Add source to the executable
    add_executable(
        ${EXEC_NAME}
        libtins.cpp
    )

    # Link the executable to the library
    target_link_libraries(${EXEC_NAME} ${LIBTINS_LIBRARIES} vifacecpp)
else()
    message(WARNING "libtins not found, not building libtins example...")
endif()

# Set dependency with the library target
add_dependencies(${PROJECT_NAME} ${EXEC_NAME})
