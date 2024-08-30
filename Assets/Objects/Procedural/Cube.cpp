#include "Cube.hpp"

Dazzle::Cube::Cube(float length)
{
    // Half of the length of a cube's side.
    const float s = length / 2.0f;
    mVertices = 
    {
        // Front face
        -s, -s,  s,
         s, -s,  s,
         s,  s,  s,
        -s,  s,  s,

        // Back face
         s, -s, -s,
        -s, -s, -s,
        -s,  s, -s,
         s,  s, -s,

        // Left face
        -s, -s, -s,
        -s, -s,  s,
        -s,  s,  s,
        -s,  s, -s,

        // Right face
         s, -s,  s,
         s, -s, -s,
         s,  s, -s,
         s,  s,  s,

        // Top face
        -s,  s,  s,
         s,  s,  s,
         s,  s, -s,
        -s,  s, -s,

        // Bottom face
        -s, -s, -s,
         s, -s, -s,
         s, -s,  s,
        -s, -s,  s,
    };

    mNormals = 
    {
        // Front face
         0.0f,  0.0f,  1.0f,
         0.0f,  0.0f,  1.0f,
         0.0f,  0.0f,  1.0f,
         0.0f,  0.0f,  1.0f,

        // Back face
         0.0f,  0.0f, -1.0f,
         0.0f,  0.0f, -1.0f,
         0.0f,  0.0f, -1.0f,
         0.0f,  0.0f, -1.0f,

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

        // Top face
         0.0f,  1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,
         0.0f,  1.0f,  0.0f,

        // Bottom face
         0.0f, -1.0f,  0.0f,
         0.0f, -1.0f,  0.0f,
         0.0f, -1.0f,  0.0f,
         0.0f, -1.0f,  0.0f,
    };

    mTextureCoordinates =
    {
        // Front face
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        // Back face
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

        // Top face
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        // Bottom face
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
    };

    // Indices (using counter-clockwise winding for front face)
    mIndices =
    {
        // Front face
         0,  1,  2,
         2,  3,  0,
        // Back face
         4,  5,  6,
         6,  7,  4,
        // Left face
         8,  9, 10,
        10, 11,  8,
        // Right face
        12, 13, 14,
        14, 15, 12,
        // Top face
        16, 17, 18,
        18, 19, 16,
        // Bottom face
        20, 21, 22,
        22, 23, 20,
    };
}
