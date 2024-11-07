#include <iostream>
#include <cmath>

#include <glm/gtc/constants.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "GL/gl3w.h"

#include "RenderSystem.hpp"
#include "Torus.hpp"

Dazzle::Torus::Torus(float majorRadius, float minorRadius, unsigned int ringSegments, unsigned int tubeSegments)
                        : mVAO(nullptr), mVBO(nullptr), mNVBO(nullptr), mEBO(nullptr)
{
    // Calculate the number of indices and vertices according to ringSegments and tubeSegments
    unsigned int vertices = ringSegments * tubeSegments;
    unsigned int indices  = vertices * 6;

    // Resize vectors according to the number of vertices and indices
    mVertices.resize(vertices * 3);
    mNormals.resize(vertices * 3);
    mTextureCoordinates.resize(vertices * 2);
    mIndices.resize(indices);

    // Calculate the angle factor for ring and tube steps
    float ringFactor = glm::two_pi<float>() / ringSegments;
    float tubeFactor = glm::two_pi<float>() / tubeSegments;

    // Generate Vertices, Normals and Texture Coordinates
    unsigned int vertexIndex = 0;
    unsigned int textureCoordIndex = 0;
    for (unsigned int i = 0; i < ringSegments; ++i)
    {
        float theta = i * ringFactor;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (unsigned int j = 0; j < tubeSegments; ++j)
        {
            float phi = j * tubeFactor;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            // Vertex Position
            mVertices[vertexIndex]      = (majorRadius + minorRadius * cosPhi) * cosTheta;
            mVertices[vertexIndex + 1]  = minorRadius * sinPhi;
            mVertices[vertexIndex + 2]  = (majorRadius + minorRadius * cosPhi) * sinTheta;

            // Normal Vector
            mNormals[vertexIndex]       = cosPhi * cosTheta;
            mNormals[vertexIndex + 1]   = sinPhi;
            mNormals[vertexIndex + 2]   = cosPhi * sinTheta;

            // Normalize Normal Vectors
            float len = sqrt( mNormals[vertexIndex] * mNormals[vertexIndex] +
                              mNormals[vertexIndex + 1] * mNormals[vertexIndex + 1] +
                              mNormals[vertexIndex + 2] * mNormals[vertexIndex + 2] );

            mNormals[vertexIndex] /= len;
            mNormals[vertexIndex + 1] /= len;
            mNormals[vertexIndex + 2] /= len;

            vertexIndex += 3;

            // Texture Coordinates
            mTextureCoordinates[textureCoordIndex]       = i / (float)ringSegments;
            mTextureCoordinates[textureCoordIndex + 1]   = j / (float)tubeSegments;

            textureCoordIndex += 2;
        }
    }

    // Generate Indices
    unsigned int index = 0;
    for (unsigned int i = 0; i < ringSegments; ++i)
    {
        for (unsigned int j = 0; j < tubeSegments; ++j)
        {
            // Indices calculation using clockwise winding order
            unsigned int first = (i * tubeSegments + j);
            unsigned int second = ((i + 1) % ringSegments) * tubeSegments + j;
            unsigned int firstNextTube = (i * tubeSegments) + ((j + 1) % tubeSegments);
            unsigned int secondNextTube = ((i + 1) % ringSegments) * tubeSegments + ((j + 1) % tubeSegments);

            // First triangle
            mIndices[index] = first;
            mIndices[index + 1] = second;
            mIndices[index + 2] = firstNextTube;
            // Second triangle
            mIndices[index + 3] = firstNextTube;
            mIndices[index + 4] = second;
            mIndices[index + 5] = secondNextTube;

            index += 6;
        }
    }

    // Transform
    mTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
}

void Dazzle::Torus::Draw() const
{
    if (!mVAO->IsValid())
        return;

    glBindVertexArray(mVAO->GetHandle());

    // Cull inner faces
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Draw
    glDrawElements(GL_TRIANGLES, (GLsizei)GetIndices().size(), GL_UNSIGNED_INT, 0);
}

void Dazzle::Torus::Rotate(glm::vec3 axis, float degrees)
{
    auto radians = glm::radians<float>(degrees);
    mTransform = glm::rotate(mTransform, radians, axis);
}

void Dazzle::Torus::InitializeBuffers()
{
    const GLintptr kOffset = 0;
    const GLintptr kStride = 0;
    const GLuint kSize = 3; // The number of values per vertex that are stored in the array.
    const GLenum kDataType = GL_FLOAT;
    const GLboolean kNormalized = GL_FALSE;

    const GLuint kPosAttribIndex = 0;
    const GLuint kPosBindingIndex = 0;
    const GLuint kNormAttribIndex = 1;
    const GLuint kNormBindingIndex = 1;

    mVAO = std::make_unique<RenderSystem::GL::VAO>();
    mVBO = std::make_unique<RenderSystem::GL::VBO>();
    mNVBO = std::make_unique<RenderSystem::GL::VBO>();
    mEBO = std::make_unique<RenderSystem::GL::EBO>();

    // Set up the data store for the Vertex Buffer Object for Vertices
    glNamedBufferStorage(mVBO->GetHandle(), GetVertices().size() * sizeof(float), GetVertices().data(), 0);

    // Set up the data store for the Vertex Buffer Object for Normals
    glNamedBufferStorage(mNVBO->GetHandle(), GetNormals().size() * sizeof(float), GetNormals().data(), 0);

    // Set up the data store for the Element Buffer Object for Indices
    glNamedBufferStorage(mEBO->GetHandle(), GetIndices().size() * sizeof(unsigned int), GetIndices().data(), 0);

    // Bind Vertex Buffer Object for Vertices to Vertex Array Object
    glVertexArrayVertexBuffer(  mVAO->GetHandle(), kPosBindingIndex, mVBO->GetHandle(), kOffset, 3 * sizeof(float));

    // Specify the format for the Vertices attribute
    glVertexArrayAttribFormat(  mVAO->GetHandle(), kPosAttribIndex, kSize, kDataType, kNormalized, kOffset);
    glVertexArrayAttribBinding( mVAO->GetHandle(), kPosAttribIndex, kPosBindingIndex);
    glEnableVertexArrayAttrib(  mVAO->GetHandle(), kPosAttribIndex);

    // Bind Vertex Buffer Object for Normals to Vertex Array Object
    glVertexArrayVertexBuffer(  mVAO->GetHandle(), kNormBindingIndex, mNVBO->GetHandle(), kOffset, 3 * sizeof(float));

    // Specify the format for the Normals attribute
    glVertexArrayAttribFormat(  mVAO->GetHandle(), kNormAttribIndex, kSize, kDataType, kNormalized, kOffset);
    glVertexArrayAttribBinding( mVAO->GetHandle(), kNormAttribIndex, kNormBindingIndex);
    glEnableVertexArrayAttrib(  mVAO->GetHandle(), kNormAttribIndex);

    // Bind Element Buffer Object to the element array buffer bind point of the Vertex Array Object
    glVertexArrayElementBuffer(mVAO->GetHandle(), mEBO->GetHandle());

    glBindVertexArray(mVAO->GetHandle());
}