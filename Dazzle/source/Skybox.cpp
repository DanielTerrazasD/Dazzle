#include <glm/gtc/constants.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "GL/gl3w.h"

#include "RenderSystem.hpp"
#include "Skybox.hpp"

Dazzle::Skybox::Skybox(float length) : mVAO(nullptr), mVBO(nullptr), mEBO(nullptr)
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

    // Indices (using counter-clockwise winding for front face)
    mIndices =
    {
        // Front face
         0,  2,  1,
         2,  0,  3,
        // Back face
         4,  6,  5,
         6,  4,  7,
        // Left face
         8, 10,  9,
        10,  8, 11,
        // Right face
        12, 14, 13,
        14, 12, 15,
        // Top face
        16, 18, 17,
        18, 16, 19,
        // Bottom face
        20, 22, 21,
        22, 20, 23,
    };

    // Transform
    mTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
}

void Dazzle::Skybox::Draw() const
{
    if (!mVAO->IsValid())
        return;

    // Draw
    glBindVertexArray(mVAO->GetHandle());
    glDrawElements(GL_TRIANGLES, (GLsizei)GetIndices().size(), GL_UNSIGNED_INT, 0);
}

void Dazzle::Skybox::SetPosition(glm::vec3 position)
{
    mTransform[3] = glm::vec4(position, 1.0f);
}

void Dazzle::Skybox::Translate(glm::vec3 position)
{
    mTransform = glm::translate(mTransform, position);
}

void Dazzle::Skybox::Rotate(glm::vec3 axis, float degrees)
{
    auto radians = glm::radians<float>(degrees);
    mTransform = glm::rotate(mTransform, radians, axis);
}

void Dazzle::Skybox::InitializeBuffers()
{
    const GLintptr kOffset = 0;
    const GLintptr kStride = 0;
    const GLuint kSize = 3; // The number of values per vertex that are stored in the array.
    const GLenum kDataType = GL_FLOAT;
    const GLboolean kNormalized = GL_FALSE;

    const GLuint kPosAttribIndex = 0;
    const GLuint kPosBindingIndex = 0;

    mVAO = std::make_unique<RenderSystem::GL::VAO>();
    mVBO = std::make_unique<RenderSystem::GL::VBO>();
    mEBO = std::make_unique<RenderSystem::GL::EBO>();

    // Set up the data store for the Vertex Buffer Object
    glNamedBufferStorage(mVBO->GetHandle(), GetVertices().size() * sizeof(float), GetVertices().data(), 0);

    // Set up the data store for the Element Buffer Object
    glNamedBufferStorage(mEBO->GetHandle(), GetIndices().size() * sizeof(unsigned int), GetIndices().data(), 0);

    glVertexArrayVertexBuffer(  mVAO->GetHandle(), kPosBindingIndex, mVBO->GetHandle(), kOffset, 3 * sizeof(float));
    glVertexArrayAttribFormat(  mVAO->GetHandle(), kPosAttribIndex, kSize, kDataType, kNormalized, kOffset);
    glVertexArrayAttribBinding( mVAO->GetHandle(), kPosAttribIndex, kPosBindingIndex);
    glEnableVertexArrayAttrib(  mVAO->GetHandle(), kPosAttribIndex);

    // Bind Element Buffer Object to the element array buffer bind point of the Vertex Array Object
    glVertexArrayElementBuffer(mVAO->GetHandle(), mEBO->GetHandle());

    glBindVertexArray(mVAO->GetHandle());
}