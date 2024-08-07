# Set the minimum version of cmake that can be used
cmake_minimum_required(VERSION 3.10)

# Set the project name
# Define project name variable
set(PROJECT_NAME Dazzle)
project (${PROJECT_NAME})

# Required packages
find_package(OpenGL REQUIRED)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
    source/main.cpp
    source/RenderSystem.cpp
    source/WindowSystem.cpp
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
    include/RenderSystem.hpp
    include/WindowSystem.hpp
)

# Add an executable with the above sources
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

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
)
