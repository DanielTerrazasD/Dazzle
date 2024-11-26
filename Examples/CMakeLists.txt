# Static library common for all the examples
add_subdirectory( Common ) 

# List of included examples
add_subdirectory(DebugShaders)
add_subdirectory(DiffuseShading)
add_subdirectory(PhongModel)
add_subdirectory(TwoSidedShading)
add_subdirectory(FlatShading)
add_subdirectory(DiscardFragments)
add_subdirectory(DirectionalLight)
add_subdirectory(PerFragmentShading)
add_subdirectory(BlinnPhongModel)
add_subdirectory(SpotLight)
add_subdirectory(ToonShading)
add_subdirectory(Fog)
add_subdirectory(PBR)
add_subdirectory(Texture)
add_subdirectory(MultipleTextures)

# Create a group of Examples
source_group(   TREE ${CMAKE_SOURCE_DIR}/Examples
                PREFIX "Examples")