#include <glm/gtc/matrix_transform.hpp>

#include <GL/gl3w.h>

#include "RenderSystem.hpp"
#include "Cube.hpp"
#include "Utilities.hpp"

Dazzle::Cube::Cube(float length) : mVAO(nullptr), mVBO(nullptr), mEBO(nullptr)
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

    // Transform
    mTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
}

void Dazzle::Cube::Draw() const
{
    if (mVAO)
    {
        // Bind Vertex Array Object
        glBindVertexArray(mVAO->GetHandle());

        // Cull inner faces
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        // Draw
        glDrawElements(GL_TRIANGLES, (GLsizei)GetIndices().size(), GL_UNSIGNED_INT, 0);

        // It's probably a good idea to unbind the VAO, to prevent making accidental changes to the
        // bound VAO in other places. However the driver complaints about a performance hit.
        // glBindVertexArray(0);
    }
}

void Dazzle::Cube::Rotate(glm::vec3 axis, float degrees)
{
    auto radians = Utilities::DegreesToRadians(degrees);
    mTransform = glm::rotate(mTransform, radians, axis);
}

void Dazzle::Cube::SetUpBuffers(unsigned int bindingIndex, unsigned int attributeIndex)
{
    const GLintptr kOffset = 0;
    const GLintptr kStride = 0;
    // const GLuint kBindingIndex = 0;  // Binding Index (from 0 to GL_MAX_VERTEX_ATTRIB_BINDINGS)
    // const GLuint kAttribIndex = 0;   // Vertex Attribute Index
    const GLuint kSize = 3; // The number of values per vertex that are stored in the array.
    const GLenum kDataType = GL_FLOAT;
    const GLboolean kNormalized = GL_FALSE;

    mVAO = std::make_unique<RenderSystem::GL::VAO>();
    mVBO = std::make_unique<RenderSystem::GL::VBO>();
    mEBO = std::make_unique<RenderSystem::GL::EBO>();

    // Set up the data store for the Vertex Buffer Object
    glNamedBufferStorage(mVBO->GetHandle(), GetVertices().size() * sizeof(float), GetVertices().data(), 0);

    // Set up the data store for the Element Buffer Object
    glNamedBufferStorage(mEBO->GetHandle(), GetIndices().size() * sizeof(unsigned int), GetIndices().data(), 0);

    // Bind Vertex Buffer Object to Vertex Array Object
    glVertexArrayVertexBuffer(  mVAO->GetHandle(),           // Vertex Array Object
                                bindingIndex,              // Binding Index
                                mVBO->GetHandle(),           // Vertex Buffer Object
                                kOffset,                    // Offset (Offset of the first element)
                                3 * sizeof(float));         // Stride (Distance between elements within the buffer)

    // Specify the format for the given attribute
    glVertexArrayAttribFormat(  mVAO->GetHandle(),           // Vertex Array Object
                                attributeIndex,               // (Vertex) Attribute Index
                                kSize,                      // Size (Number of values per vertex that are stored in the array)
                                kDataType,                  // Data Type
                                kNormalized,                // Normalized (if parameter represents a normalized integer)
                                kOffset);                   // Relative Offset

    // Specify which vertex buffer binding to use for this attribute
    glVertexArrayAttribBinding( mVAO->GetHandle(),           // Vertex Array Object
                                attributeIndex,               // (Vertex) Attribute Index
                                bindingIndex);             // Binding Index

    // Enable the attribute
    glEnableVertexArrayAttrib(  mVAO->GetHandle(),           // Vertex Array Object
                                attributeIndex);              // (Vertex) Attribute Index

    // Bind Element Buffer Object to the element array buffer bind point of the Vertex Array Object
    glVertexArrayElementBuffer(mVAO->GetHandle(), mEBO->GetHandle());
}