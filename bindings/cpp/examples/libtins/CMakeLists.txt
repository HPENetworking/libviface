include(FindPkgConfig)

pkg_check_modules(LIBTINS "libtins")

if(LIBTINS_FOUND)
    set(EXEC_NAME "libtins")

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
        ${EXEC_NAME}.cpp
    )

    # Link the executable to the library
    target_link_libraries(${EXEC_NAME} ${LIBTINS_LIBRARIES} viface)
else()
    message(WARNING "libtins not found, not building libtins example...")
endif()