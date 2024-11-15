#include <glm/gtc/constants.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "GL/gl3w.h"

#include "RenderSystem.hpp"
#include "Cube.hpp"

Dazzle::Cube::Cube(float length) : mVAO(nullptr), mVBO(nullptr), mNVBO(nullptr), mTCVBO(nullptr), mEBO(nullptr)
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
    if (!mVAO->IsValid())
        return;

    // Draw
    glBindVertexArray(mVAO->GetHandle());
    glDrawElements(GL_TRIANGLES, (GLsizei)GetIndices().size(), GL_UNSIGNED_INT, 0);

    // It's probably a good idea to unbind the VAO, to prevent making accidental changes to the
    // bound VAO in other places. However the driver complaints about a performance hit.
    // glBindVertexArray(0);
}

void Dazzle::Cube::Rotate(glm::vec3 axis, float degrees)
{
    auto radians = glm::radians<float>(degrees);
    mTransform = glm::rotate(mTransform, radians, axis);
}

void Dazzle::Cube::InitializeBuffers()
{
    const GLintptr kOffset = 0;
    const GLintptr kStride = 0;
    const GLuint kSize = 3; // The number of values per vertex that are stored in the array.
    const GLuint kTexSize = 2; // Number of values per vertex texture coordinate
    const GLenum kDataType = GL_FLOAT;
    const GLboolean kNormalized = GL_FALSE;

    const GLuint kPosAttribIndex = 0;
    const GLuint kPosBindingIndex = 0;
    const GLuint kNormAttribIndex = 1;
    const GLuint kNormBindingIndex = 1;
    const GLuint kTexAttribIndex = 2;
    const GLuint kTexBindingIndex = 2;

    mVAO = std::make_unique<RenderSystem::GL::VAO>();
    mVBO = std::make_unique<RenderSystem::GL::VBO>();
    mNVBO = std::make_unique<RenderSystem::GL::VBO>();
    mTCVBO = std::make_unique<RenderSystem::GL::VBO>();
    mEBO = std::make_unique<RenderSystem::GL::EBO>();

    // Set up the data store for the Vertex Buffer Object
    glNamedBufferStorage(mVBO->GetHandle(), GetVertices().size() * sizeof(float), GetVertices().data(), 0);

    // Set up the data store for the Vertex Buffer Object for Normals
    glNamedBufferStorage(mNVBO->GetHandle(), GetNormals().size() * sizeof(float), GetNormals().data(), 0);

    // Set up the data store for the Vertex Buffer Object for Texture Coordinates
    glNamedBufferStorage(mTCVBO->GetHandle(), GetTextureCoordinates().size() * sizeof(float), GetTextureCoordinates().data(), 0);

    // Set up the data store for the Element Buffer Object
    glNamedBufferStorage(mEBO->GetHandle(), GetIndices().size() * sizeof(unsigned int), GetIndices().data(), 0);

    // Bind Vertex Buffer Object to Vertex Array Object
    glVertexArrayVertexBuffer(  mVAO->GetHandle(),          // Vertex Array Object
                                kPosBindingIndex,           // Binding Index
                                mVBO->GetHandle(),          // Vertex Buffer Object
                                kOffset,                    // Offset (Offset of the first element)
                                3 * sizeof(float));         // Stride (Distance between elements within the buffer)

    // Specify the format for the given attribute
    glVertexArrayAttribFormat(  mVAO->GetHandle(),          // Vertex Array Object
                                kPosAttribIndex,            // (Vertex) Attribute Index
                                kSize,                      // Size (Number of values per vertex that are stored in the array)
                                kDataType,                  // Data Type
                                kNormalized,                // Normalized (if parameter represents a normalized integer)
                                kOffset);                   // Relative Offset

    // Specify which vertex buffer binding to use for this attribute
    glVertexArrayAttribBinding( mVAO->GetHandle(),          // Vertex Array Object
                                kPosAttribIndex,            // (Vertex) Attribute Index
                                kPosBindingIndex);          // Binding Index

    // Enable the attribute
    glEnableVertexArrayAttrib(  mVAO->GetHandle(),          // Vertex Array Object
                                kPosAttribIndex);           // (Vertex) Attribute Index

    // Bind Vertex Buffer Object for Normals to Vertex Array Object
    glVertexArrayVertexBuffer(  mVAO->GetHandle(), kNormBindingIndex, mNVBO->GetHandle(), kOffset, 3 * sizeof(float));

    // Specify the format for the Normals attribute
    glVertexArrayAttribFormat(  mVAO->GetHandle(), kNormAttribIndex, kSize, kDataType, kNormalized, kOffset);
    glVertexArrayAttribBinding( mVAO->GetHandle(), kNormAttribIndex, kNormBindingIndex);
    glEnableVertexArrayAttrib(  mVAO->GetHandle(), kNormAttribIndex);

    // Bind Vertex Buffer Object for the Texture Coordinates to Vertex Array Object
    glVertexArrayVertexBuffer(  mVAO->GetHandle(), kTexBindingIndex, mTCVBO->GetHandle(), kOffset, 2 * sizeof(float));

    // Specify the format for the Normals attribute
    glVertexArrayAttribFormat(  mVAO->GetHandle(), kTexAttribIndex, kTexSize, kDataType, kNormalized, kOffset);
    glVertexArrayAttribBinding( mVAO->GetHandle(), kTexAttribIndex, kTexBindingIndex);
    glEnableVertexArrayAttrib(  mVAO->GetHandle(), kTexAttribIndex);

    // Bind Element Buffer Object to the element array buffer bind point of the Vertex Array Object
    glVertexArrayElementBuffer(mVAO->GetHandle(), mEBO->GetHandle());

    // According to:
    // https://www.khronos.org/opengl/wiki/Shader_Compilation#Program_setup
    // "A number of parameters can be set up that will affect the linking process. This generally involves interfaces with the program. These include:"
    // 1.Vertex shader input attribute locations.
    // 2.Fragment shader output color numbers.
    // 3. Transform feedback output capturing.
    // 4. Program separation.
    // "You cannot change these values after linking; if you don't set them before linking, you can't set them at all."
    
    // Thus, this cause the following warning call to the DebugMessageCallback:
    // "Program/shader state performance warning: Vertex shader in program X is being recompiled based on GL state.""

    // Since, DSA is being used to configure the VAO, there isn't anything bound to the state machine.
    // So, a binding call should be made like: glEnableVertexAttribArray or glBindVertexArray. Before the linking process.
    glBindVertexArray(mVAO->GetHandle());
}