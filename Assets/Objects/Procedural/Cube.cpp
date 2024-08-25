#include "Cube.hpp"

Dazzle::Cube::Cube(float length)
{
    // Half of the length of a cube's side.
    const float s = length / 2.0f;
    mVertices = 
    {
        // Back face
        -s, -s, -s,
         s, -s, -s,
         s,  s, -s,
        -s,  s, -s,

        // Front face
        -s, -s,  s,
         s, -s,  s,
         s,  s,  s,
        -s,  s,  s,

        // Left face
        -s,  s,  s,
        -s,  s, -s,
        -s, -s, -s,
        -s, -s,  s,

        // Right face
         s,  s,  s,
         s,  s, -s,
         s, -s, -s,
         s, -s,  s,

        // Bottom face
        -s, -s, -s,
         s, -s, -s,
         s, -s,  s,
        -s, -s,  s,

        // Top face
        -s,  s, -s,
         s,  s, -s,
         s,  s,  s,
        -s,  s,  s,
    };

    mNormals = 
    {
        // Back face
         0.0f,  0.0f, -1.0f,
         0.0f,  0.0f, -1.0f,
         0.0f,  0.0f, -1.0f,
         0.0f,  0.0f, -1.0f,

        // Front face
         0.0f,  0.0f,  1.0f,
         0.0f,  0.0f,  1.0f,
         0.0f,  0.0f,  1.0f,
         0.0f,  0.0f,  1.0f,

        // Left face
        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,
        -1.0f,  0.0f,  0.0f,

        // Right face
         1.0f,  0.0f,  0.0f,
         1.0f,  0.0f,  0.0f,
         1.0f,  0.0f,  0.0f,
         1.0f,  0.0f,  0.0f,

        // Bottom face
         0.0f, -1.0f,  0.0f,
         0.0f, -1.0f,  0.0f,
         0.0f, -1.0f,  0.0f,
         0.0f, -1.0f,  0.0f,

        // Top face
         0.0f,  1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,
    };

    mTextureCoordinates =
    {
        // Back face
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        // Front face
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        // Left face
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

        // Right face
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

        // Bottom face
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        // Top face
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
    };

    // Indices (using counter-clockwise winding for front face)
    mIndices =
    {
        0,  1,  2,  2,  3,  0,    // Back face
        4,  5,  6,  6,  7,  4,    // Front face
        8,  9, 10, 10, 11,  8,    // Left face
       12, 13, 14, 14, 15, 12,    // Right face
       16, 17, 18, 18, 19, 16,    // Bottom face
       20, 21, 22, 22, 23, 20     // Top face
    };
}
