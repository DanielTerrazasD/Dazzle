set(TARGET gl3w)

# Required packages
find_package(OpenGL REQUIRED)

set(SOURCES
    ${CMAKE_SOURCE_DIR}/Vendor/gl3w/src/gl3w.c
    ${CMAKE_SOURCE_DIR}/Vendor/gl3w/include/GL/gl3w.h
    ${CMAKE_SOURCE_DIR}/Vendor/gl3w/include/GL/glcorearb.h
    ${CMAKE_SOURCE_DIR}/Vendor/gl3w/include/KHR/khrplatform.h
)

add_library(${TARGET} STATIC ${SOURCES})

# Add this project to the "Libraries" folder.
set_target_properties(${TARGET} PROPERTIES FOLDER "Libraries")

# Link required libraries
target_link_libraries(${TARGET} PUBLIC OpenGL::GL glfw)

target_include_directories(${TARGET}
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/Vendor/gl3w/include
)