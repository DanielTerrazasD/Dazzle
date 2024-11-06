# Static library common for all the examples
add_subdirectory( Common ) 

# List of included Examples
add_subdirectory(DiffuseShader)
add_subdirectory(PhongModel)

# Create a group of Examples
source_group(   TREE ${CMAKE_SOURCE_DIR}/Examples
                PREFIX "Examples")