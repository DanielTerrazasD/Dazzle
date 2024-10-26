add_subdirectory(gl3w)
# TODO: add_subdirectory(glfw)
add_subdirectory(imgui)

# Create a group of Libraries
source_group(   TREE ${CMAKE_SOURCE_DIR}/Libraries
                PREFIX "Libraries")