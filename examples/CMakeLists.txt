
# List of included Examples
add_subdirectory(_glfw_and_opengl3)
add_subdirectory(_sandbox)
add_subdirectory(_win32_and_opengl3)


# TODO - Static Library: add_subdirectory( Common ) 
add_subdirectory(DiffuseShader)

# Create a group of Examples
source_group(   TREE ${CMAKE_SOURCE_DIR}/Examples
                PREFIX "Examples")