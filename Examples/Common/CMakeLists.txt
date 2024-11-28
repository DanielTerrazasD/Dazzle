set(TARGET Common)

# Required packages
find_package(OpenGL REQUIRED)

set(SOURCES
    App.cpp
    App.hpp
    Camera.cpp
    Camera.hpp
    Cursor.cpp
    Cursor.hpp
    CursorInputMode.cpp
    CursorInputMode.hpp
    Framebuffer.cpp
    Framebuffer.hpp
    Keyboard.cpp
    Keyboard.hpp
    pch.hpp
    Scene.hpp
    UserInterface.cpp
    UserInterface.hpp
    Utils.cpp
    Utils.hpp
    Window.cpp
    Window.hpp
)

add_library(${TARGET} STATIC ${SOURCES})

# Add this project to the "Examples" folder.
set_target_properties(${TARGET} PROPERTIES FOLDER "Examples")

# Ensure that dependencies are compiled first
add_dependencies(${TARGET} gl3w imgui Common)

# Link required libraries
target_link_libraries(${TARGET} PUBLIC OpenGL::GL glfw)

target_include_directories(${TARGET}
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/Vendor/imgui
        ${CMAKE_SOURCE_DIR}/Vendor/imgui/backends
        ${CMAKE_SOURCE_DIR}/Vendor/gl3w/include
        ${CMAKE_SOURCE_DIR}/Vendor/glm
        ${CMAKE_SOURCE_DIR}/Vendor/tinyobjloader
)