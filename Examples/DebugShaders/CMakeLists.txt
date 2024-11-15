# Set the minimum version of cmake that can be used
cmake_minimum_required(VERSION 3.10)

# Set the project name
set(TARGET DebugShaders)

# Required packages
find_package(OpenGL REQUIRED)

# Create a sources variable with a link to all cpp files to compile
set(SOURCES
    main.cpp
    ${CMAKE_SOURCE_DIR}/Dazzle/source/FileManager.cpp
    ${CMAKE_SOURCE_DIR}/Dazzle/source/RenderSystem.cpp
    ${CMAKE_SOURCE_DIR}/Dazzle/source/Cube.cpp
    ${CMAKE_SOURCE_DIR}/Dazzle/source/Plane.cpp
    ${CMAKE_SOURCE_DIR}/Dazzle/source/Sphere.cpp
    ${CMAKE_SOURCE_DIR}/Dazzle/source/Torus.cpp
)

set(HEADERS
    ${CMAKE_SOURCE_DIR}/Dazzle/include/FileManager.hpp
    ${CMAKE_SOURCE_DIR}/Dazzle/include/Object3D.hpp
    ${CMAKE_SOURCE_DIR}/Dazzle/include/RenderSystem.hpp
    ${CMAKE_SOURCE_DIR}/Dazzle/include/Cube.hpp
    ${CMAKE_SOURCE_DIR}/Dazzle/include/Plane.hpp
    ${CMAKE_SOURCE_DIR}/Dazzle/include/Sphere.hpp
    ${CMAKE_SOURCE_DIR}/Dazzle/include/Torus.hpp
)

# Add an executable with the above sources
add_executable(${TARGET} ${SOURCES} ${HEADERS})

add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND xcopy /E /I /Y 
    "\"${CMAKE_CURRENT_SOURCE_DIR}/shaders\""  # Source directory
    "\"$<TARGET_FILE_DIR:${TARGET}>/shaders\"" # Destination directory
)

# Add this project to the "Examples" folder.
set_target_properties(${TARGET} PROPERTIES FOLDER "Examples")
# Set VS Debugging Working Directory equal to $(TargetPath)
set_target_properties(${TARGET} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${TARGET}>")

# Ensure that dependencies are compiled first
add_dependencies(${TARGET} gl3w imgui Common)

# Link required libraries for this target
target_link_libraries(${TARGET} PRIVATE OpenGL::GL gl3w glfw imgui Common)

# Set the directories that should be included in the build command for this target
target_include_directories(${TARGET}
    PRIVATE 
        ${CMAKE_SOURCE_DIR}/Dazzle/include
        ${CMAKE_SOURCE_DIR}/Examples/Common
        ${CMAKE_SOURCE_DIR}/Vendor/glm
)