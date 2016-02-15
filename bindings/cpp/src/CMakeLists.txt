# Define library name
set(LIB_NAME "vifacecpp")

# Add libviface library to build
add_library(${LIB_NAME} SHARED viface.cpp)

# Set library version
set_target_properties(
    ${LIB_NAME} PROPERTIES VERSION ${libvifacecpp_VERSION_STRING}
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
