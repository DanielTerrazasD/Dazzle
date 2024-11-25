#include <glm/gtc/constants.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "GL/gl3w.h"

#include "RenderSystem.hpp"
#include "Plane.hpp"

Dazzle::Plane::Plane(float width, float depth, unsigned int widthSegments, unsigned int depthSegments)
                    : mVAO(nullptr), mVBO(nullptr), mNVBO(nullptr), mTCVBO(nullptr), mEBO(nullptr)
{
    // Calculate the number of indices and vertices according to widthSegments and depthSegments
    unsigned int vertices = (widthSegments + 1) * (depthSegments + 1);
    unsigned int indices = widthSegments * depthSegments * 6;

    // Resize vectors according to the number of vertices and indices
    mVertices.resize(vertices * 3);
    mNormals.resize(vertices * 3);
    mTextureCoordinates.resize(vertices * 2);
    mIndices.resize(indices);

    float halfWidth = width / 2.0f;
    float halfDepth = depth / 2.0f;
    float widthDiff = width / widthSegments;
    float depthDiff = depth / depthSegments;

    unsigned int vertexIndex = 0;
    unsigned int textureCoordIndex = 0;

    // Generate Vertices, Normals and Texture Coordinates
    for (unsigned int i = 0; i <= depthSegments; ++i)
    {
        float z = -halfDepth + i * depthDiff;
        for (unsigned int j = 0; j <= widthSegments; ++j)
        {
            float x = -halfWidth + j * widthDiff;

            // Vertex Position
            mVertices[vertexIndex] = x;
            mVertices[vertexIndex + 1] = 0.0f;
            mVertices[vertexIndex + 2] = z;

            // Normal Vector
            mNormals[vertexIndex] = 0.0f;
            mNormals[vertexIndex + 1] = 1.0f;
            mNormals[vertexIndex + 2] = 0.0f;

            vertexIndex += 3;

            // Texture Coordinates
            mTextureCoordinates[textureCoordIndex]     = static_cast<float>(j) / widthSegments;
            mTextureCoordinates[textureCoordIndex + 1] = static_cast<float>(i) / depthSegments;
            textureCoordIndex += 2;
        }
    }

    // Generate Indices
    unsigned int index = 0;
    for (unsigned int i = 0; i < depthSegments; ++i)
    {
        for (unsigned int j = 0; j < widthSegments; ++j)
        {
            int start = i * (widthSegments + 1) + j;

            // Two triangles per quad
            mIndices[index]     = start;
            mIndices[index + 1] = start + widthSegments + 1;
            mIndices[index + 2] = start + 1;

            mIndices[index + 3] = start + 1;
            mIndices[index + 4] = start + widthSegments + 1;
            mIndices[index + 5] = start + widthSegments + 2;
            index += 6;
        }
    }

    // Transform
    mTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
}

void Dazzle::Plane::Draw() const
{
    if (!mVAO->IsValid())
        return;

    // Bind VAO
    glBindVertexArray(mVAO->GetHandle());

    // Draw
    glDrawElements(GL_TRIANGLES, (GLsizei)GetIndices().size(), GL_UNSIGNED_INT, 0);
}

void Dazzle::Plane::SetPosition(glm::vec3 position)
{
    mTransform[3] = glm::vec4(position, 1.0f);
}

void Dazzle::Plane::Translate(glm::vec3 position)
{
    mTransform = glm::translate(mTransform, position);
}

void Dazzle::Plane::Rotate(glm::vec3 axis, float degrees)
{
    auto radians = glm::radians<float>(degrees);
    mTransform = glm::rotate(mTransform, radians, axis);
}

void Dazzle::Plane::InitializeBuffers()
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

    glBindVertexArray(mVAO->GetHandle());
}