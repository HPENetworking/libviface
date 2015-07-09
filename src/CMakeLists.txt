# Define library name
set(LIB_NAME "viface")

# Add libviface library to build
add_library(${LIB_NAME} SHARED viface.cpp)

# Set library version
set_target_properties(
    ${LIB_NAME} PROPERTIES VERSION ${libviface_VERSION_STRING}
)

# Link the library code all together
target_link_libraries(${LIB_NAME})

# Specify shared library install directory
install(TARGETS ${LIB_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
