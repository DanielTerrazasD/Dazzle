set(TARGET imgui)

# Required packages
find_package(OpenGL REQUIRED)

set(SOURCES
    ${CMAKE_SOURCE_DIR}/Vendor/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/Vendor/imgui/imgui.h
    ${CMAKE_SOURCE_DIR}/Vendor/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/Vendor/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/Vendor/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/Vendor/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/Vendor/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/Vendor/imgui/backends/imgui_impl_glfw.h
    ${CMAKE_SOURCE_DIR}/Vendor/imgui/backends/imgui_impl_opengl3.cpp
    ${CMAKE_SOURCE_DIR}/Vendor/imgui/backends/imgui_impl_opengl3.h
    ${CMAKE_SOURCE_DIR}/Vendor/imgui/backends/imgui_impl_opengl3_loader.h
)

add_library(${TARGET} STATIC ${SOURCES})

# Add this project to the "Libraries" folder.
set_target_properties(${TARGET} PROPERTIES FOLDER "Libraries")

# Link required libraries
target_link_libraries(${TARGET} PUBLIC OpenGL::GL glfw)

target_include_directories(${TARGET}
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/Vendor/imgui
        ${CMAKE_SOURCE_DIR}/Vendor/imgui/backends
)