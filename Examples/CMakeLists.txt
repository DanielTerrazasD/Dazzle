# Static library common for all the examples
add_subdirectory( Common ) 

# List of included Examples
add_subdirectory(DiffuseShading)
add_subdirectory(PhongModel)
add_subdirectory(TwoSidedShading)
add_subdirectory(FlatShading)
add_subdirectory(DiscardFragments)

# Create a group of Examples
source_group(   TREE ${CMAKE_SOURCE_DIR}/Examples
                PREFIX "Examples")