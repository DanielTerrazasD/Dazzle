# Set the minimum version of cmake that can be used
cmake_minimum_required(VERSION 3.10)

# Set the project name
# Define project name variable
set(PROJECT_NAME Dazzle)
project (${PROJECT_NAME})

# Set output directories for different build types
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/bin/$<CONFIG>")

# Required packages
find_package(OpenGL REQUIRED)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
    source/main.cpp
    source/FileManager.cpp
    source/RenderSystem.cpp
    source/ShaderManager.cpp
    source/ShadingEffects.cpp
    ${CMAKE_SOURCE_DIR}/Assets/Objects/Procedural/Cube.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/vendor/gl3w/src/gl3w.c
    )

set(HEADERS
    include/DazzleEngine.hpp
    include/FileManager.hpp
    include/Singleton.hpp
    include/ShaderManager.hpp
    include/ShadingEffects.hpp
    include/RenderSystem.hpp
    ${CMAKE_SOURCE_DIR}/Assets/Objects/Procedural/Cube.hpp
    )

# Add an executable with the above sources
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Configure the Debugging Working Directory
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

# Specify GLFW_INCLUDE_NONE=1 to not include GLFW any OpenGL headers
target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)
# Specify IMGUI_IMPL_OPENGL_LOADER_CUSTOM=1 to use gl3w from vendor's folder instead
target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_IMPL_OPENGL_LOADER_CUSTOM)

# Link required libraries for this target
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

# Set the directories that should be included in the build command for this target
target_include_directories(${PROJECT_NAME}
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/vendor/imgui
        ${CMAKE_SOURCE_DIR}/vendor/imgui/backends
        ${CMAKE_SOURCE_DIR}/vendor/gl3w/include
        ${CMAKE_SOURCE_DIR}/vendor/glm
        ${CMAKE_SOURCE_DIR}/Assets/Objects/Procedural
)

# Add a post-build custom command to copy the Assets into the output directory.
set(ASSETS_SOURCE "${CMAKE_SOURCE_DIR}/Assets")
set(ASSETS_DEST "${CMAKE_BINARY_DIR}/Assets")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSETS_SOURCE} ${ASSETS_DEST}
    COMMENT "Copying files from ${ASSETS_SOURCE} to ${ASSETS_DEST}"
)