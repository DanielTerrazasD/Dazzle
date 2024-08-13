
# List of included examples
add_subdirectory(_mahogany_ui)
add_subdirectory(_sandbox)
add_subdirectory(glfw_and_opengl3)
add_subdirectory(win32_and_opengl3)

# Create a group of examples
source_group(   TREE ${CMAKE_SOURCE_DIR}/examples
                PREFIX "examples")