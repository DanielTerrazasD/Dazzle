# Set the minimum version of cmake that can be used
cmake_minimum_required(VERSION 3.10)

# Set the project name
set(PROJECT_NAME DiffuseShader)
project (${PROJECT_NAME})

# Required packages
find_package(OpenGL REQUIRED)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
    source/main.cpp
    ${CMAKE_SOURCE_DIR}/Dazzle/source/Cube.cpp
    ${CMAKE_SOURCE_DIR}/Dazzle/source/FileManager.cpp
    ${CMAKE_SOURCE_DIR}/Dazzle/source/RenderSystem.cpp
    ${CMAKE_SOURCE_DIR}/examples/common/App.cpp
    ${CMAKE_SOURCE_DIR}/examples/common/Camera.cpp
    ${CMAKE_SOURCE_DIR}/examples/common/UserInterface.cpp
    ${CMAKE_SOURCE_DIR}/examples/common/Window.cpp
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
    ${CMAKE_SOURCE_DIR}/Dazzle/include/Cube.hpp
    ${CMAKE_SOURCE_DIR}/Dazzle/include/FileManager.hpp
    ${CMAKE_SOURCE_DIR}/Dazzle/include/Object3D.hpp
    ${CMAKE_SOURCE_DIR}/Dazzle/include/RenderSystem.hpp
    ${CMAKE_SOURCE_DIR}/Dazzle/include/Utilities.hpp
    ${CMAKE_SOURCE_DIR}/examples/common/pch.hpp
    ${CMAKE_SOURCE_DIR}/examples/common/App.hpp
    ${CMAKE_SOURCE_DIR}/examples/common/Camera.hpp
    ${CMAKE_SOURCE_DIR}/examples/common/Scene.hpp
    ${CMAKE_SOURCE_DIR}/examples/common/UserInterface.hpp
    ${CMAKE_SOURCE_DIR}/examples/common/Window.hpp
)

# Add an executable with the above sources
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Add this project to the "examples" folder.
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "examples")

# Specify GLFW_INCLUDE_NONE=1 to not include GLFW any OpenGL headers
# target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)
# Specify IMGUI_IMPL_OPENGL_LOADER_CUSTOM=1 to use gl3w from vendor's folder instead
# target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_IMPL_OPENGL_LOADER_CUSTOM)

# Link required libraries for this target
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

# Set the directories that should be included in the build command for this target
target_include_directories(${PROJECT_NAME}
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/Dazzle/include
        ${CMAKE_SOURCE_DIR}/Examples/Common
        ${CMAKE_SOURCE_DIR}/Vendor/imgui
        ${CMAKE_SOURCE_DIR}/Vendor/imgui/backends
        ${CMAKE_SOURCE_DIR}/Vendor/gl3w/include
        ${CMAKE_SOURCE_DIR}/Vendor/glm
)

FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})