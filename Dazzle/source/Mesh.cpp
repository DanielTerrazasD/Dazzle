#include <glm/gtc/constants.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "GL/gl3w.h"

#include "RenderSystem.hpp"
#include "Mesh.hpp"

Dazzle::Mesh::Mesh() : mTransform(glm::mat4(1.0))
{

}

void Dazzle::Mesh::Draw() const
{
    if (mVAO == nullptr || !mVAO->IsValid())
        return;

    // Draw
    glBindVertexArray(mVAO->GetHandle());
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)GetVertices().size());
}

void Dazzle::Mesh::SetPosition(glm::vec3 position)
{
    mTransform[3] = glm::vec4(position, 1.0f);
}

void Dazzle::Mesh::Translate(glm::vec3 position)
{
    mTransform = glm::translate(mTransform, position);
}

void Dazzle::Mesh::Rotate(glm::vec3 axis, float degrees)
{
    auto radians = glm::radians<float>(degrees);
    mTransform = glm::rotate(mTransform, radians, axis);
}

void Dazzle::Mesh::InitializeBuffers()
{
    const GLintptr kOffset = 0;
    const GLintptr kStride = 0;
    const GLuint kSize = 3; // The number of values per vertex that are stored in the array.
    const GLuint kTexSize = 2; // Number of values per vertex texture coordinate
    const GLenum kDataType = GL_FLOAT;
    const GLboolean kNormalized = GL_FALSE;

    // Vertices
    const GLuint kPosAttribIndex = 0;
    const GLuint kPosBindingIndex = 0;
    // Normals
    const GLuint kNormAttribIndex = 1;
    const GLuint kNormBindingIndex = 1;
    // Texture Coordinates
    const GLuint kTexAttribIndex = 2;
    const GLuint kTexBindingIndex = 2;
    // Tangents
    const GLuint kTangAttribIndex = 3;
    const GLuint kTangBindingIndex = 3;
    // Bitangents
    const GLuint kBitanAttribIndex = 4;
    const GLuint kBitanBindingIndex = 4;

    if (mVertices.empty())
        return;

    mVAO = std::make_unique<RenderSystem::GL::VAO>();
    mVBO = std::make_unique<RenderSystem::GL::VBO>();

    if (!mNormals.empty())
        mNVBO = std::make_unique<RenderSystem::GL::VBO>();
    if (!mTextureCoordinates.empty())
        mTCVBO = std::make_unique<RenderSystem::GL::VBO>();
    if (!mTangents.empty())
        mTVBO = std::make_unique<RenderSystem::GL::VBO>();
    if (!mBitangents.empty())
        mBVBO = std::make_unique<RenderSystem::GL::VBO>();


    // Set up the data store for the Vertex Buffer Object
    glNamedBufferStorage(mVBO->GetHandle(), GetVertices().size() * sizeof(float), GetVertices().data(), 0);

    // Set up the data store for the Vertex Buffer Object for Normals
    if (mNVBO != nullptr)
        glNamedBufferStorage(mNVBO->GetHandle(), GetNormals().size() * sizeof(float), GetNormals().data(), 0);

    // Set up the data store for the Vertex Buffer Object for Texture Coordinates
    if (mTCVBO != nullptr)
        glNamedBufferStorage(mTCVBO->GetHandle(), GetTextureCoordinates().size() * sizeof(float), GetTextureCoordinates().data(), 0);

    // Set up the data store for the Vertex Buffer Object for Tangents
    if (mTVBO != nullptr)
        glNamedBufferStorage(mTVBO->GetHandle(), GetTangents().size() * sizeof(float), GetTangents().data(), 0);

    // Set up the data store for the Vertex Buffer Object for Bitangents
    if (mBVBO != nullptr)
        glNamedBufferStorage(mBVBO->GetHandle(), GetBitangents().size() * sizeof(float), GetBitangents().data(), 0);

    // VBO
    glVertexArrayVertexBuffer(  mVAO->GetHandle(), kPosBindingIndex, mVBO->GetHandle(), kOffset, 3 * sizeof(float));
    glVertexArrayAttribFormat(  mVAO->GetHandle(), kPosAttribIndex, kSize, kDataType, kNormalized, kOffset);
    glVertexArrayAttribBinding( mVAO->GetHandle(), kPosAttribIndex, kPosBindingIndex);
    glEnableVertexArrayAttrib(  mVAO->GetHandle(), kPosAttribIndex);

    // Normals
    if (mNVBO != nullptr)
    {
        glVertexArrayVertexBuffer(  mVAO->GetHandle(), kNormBindingIndex, mNVBO->GetHandle(), kOffset, 3 * sizeof(float));
        glVertexArrayAttribFormat(  mVAO->GetHandle(), kNormAttribIndex, kSize, kDataType, kNormalized, kOffset);
        glVertexArrayAttribBinding( mVAO->GetHandle(), kNormAttribIndex, kNormBindingIndex);
        glEnableVertexArrayAttrib(  mVAO->GetHandle(), kNormAttribIndex);
    }

    // Texture Coordinates
    if (mTCVBO != nullptr)
    {
        glVertexArrayVertexBuffer(  mVAO->GetHandle(), kTexBindingIndex, mTCVBO->GetHandle(), kOffset, 2 * sizeof(float));
        glVertexArrayAttribFormat(  mVAO->GetHandle(), kTexAttribIndex, kTexSize, kDataType, kNormalized, kOffset);
        glVertexArrayAttribBinding( mVAO->GetHandle(), kTexAttribIndex, kTexBindingIndex);
        glEnableVertexArrayAttrib(  mVAO->GetHandle(), kTexAttribIndex);
    }

    // Tangents
    if (mTVBO != nullptr)
    {
        glVertexArrayVertexBuffer(  mVAO->GetHandle(), kTangBindingIndex, mTVBO->GetHandle(), kOffset, 4 * sizeof(float));
        glVertexArrayAttribFormat(  mVAO->GetHandle(), kTangAttribIndex, kSize, kDataType, kNormalized, kOffset);
        glVertexArrayAttribBinding( mVAO->GetHandle(), kTangAttribIndex, kTangBindingIndex);
        glEnableVertexArrayAttrib(  mVAO->GetHandle(), kTangAttribIndex);
    }

    // Bitangents
    if (mBVBO != nullptr)
    {
        glVertexArrayVertexBuffer(  mVAO->GetHandle(), kBitanBindingIndex, mBVBO->GetHandle(), kOffset, 3 * sizeof(float));
        glVertexArrayAttribFormat(  mVAO->GetHandle(), kBitanAttribIndex, kSize, kDataType, kNormalized, kOffset);
        glVertexArrayAttribBinding( mVAO->GetHandle(), kBitanAttribIndex, kBitanBindingIndex);
        glEnableVertexArrayAttrib(  mVAO->GetHandle(), kBitanAttribIndex);
    }

    glBindVertexArray(mVAO->GetHandle());
}