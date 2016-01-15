# Define library name
set(LIB_NAME "viface")

# Add libviface library to build
add_library(${LIB_NAME} SHARED viface.c)

# Add APR dependencies
if (NOT APR_FOUND)
  add_dependencies(${LIB_NAME} apr_lib)
endif()
if (NOT APRUTIL_FOUND)
  add_dependencies(${LIB_NAME} aprutil_lib)
endif()

# Link target with APR libraries
target_link_libraries(${LIB_NAME} ${APR_LIBRARIES} ${APRUTIL_LIBRARIES})

# Set library version
set_target_properties(
    ${LIB_NAME} PROPERTIES VERSION ${libviface_VERSION_STRING}
)

# Specify shared library install directory
install(
    TARGETS
    ${LIB_NAME}
    DESTINATION
    "${CMAKE_INSTALL_LIBDIR}"
)

# Set dependency with the library target
add_dependencies(${PROJECT_NAME} ${LIB_NAME})
