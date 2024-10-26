# Set the minimum version of cmake that can be used
cmake_minimum_required(VERSION 3.10)

# Set the project name
set(TARGET DiffuseShader)

# Required packages
find_package(OpenGL REQUIRED)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
    source/main.cpp
    ${CMAKE_SOURCE_DIR}/Dazzle/source/Cube.cpp
    ${CMAKE_SOURCE_DIR}/Dazzle/source/FileManager.cpp
    ${CMAKE_SOURCE_DIR}/Dazzle/source/RenderSystem.cpp
)

set(HEADERS
    ${CMAKE_SOURCE_DIR}/Dazzle/include/Cube.hpp
    ${CMAKE_SOURCE_DIR}/Dazzle/include/FileManager.hpp
    ${CMAKE_SOURCE_DIR}/Dazzle/include/Object3D.hpp
    ${CMAKE_SOURCE_DIR}/Dazzle/include/RenderSystem.hpp
    ${CMAKE_SOURCE_DIR}/Dazzle/include/Utilities.hpp
)

# Add an executable with the above sources
add_executable(${TARGET} ${SOURCES} ${HEADERS})

# Add this project to the "Examples" folder.
set_target_properties(${TARGET} PROPERTIES FOLDER "Examples")

# Ensure that dependencies are compiled first
add_dependencies(${TARGET} gl3w imgui Common)


# Link required libraries for this target
target_link_libraries(${TARGET} PRIVATE OpenGL::GL gl3w glfw imgui Common)

# Set the directories that should be included in the build command for this target
target_include_directories(${TARGET}
    PRIVATE 
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/Dazzle/include
        ${CMAKE_SOURCE_DIR}/Examples/Common
        ${CMAKE_SOURCE_DIR}/Vendor/glm
)

FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})