#include <cmath>

#include <glm/gtc/constants.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include "GL/gl3w.h"

#include "RenderSystem.hpp"
#include "Sphere.hpp"

Dazzle::Sphere::Sphere(float radius, unsigned int latitudeSegments, unsigned int longitudeSegments)
                        : mVAO(nullptr), mVBO(nullptr), mEBO(nullptr)
{
    // Calculate the number of indices and vertices according to latitudeSegments and longitudeSegments
    unsigned int vertices = (latitudeSegments + 1) * (longitudeSegments + 1);
    unsigned int indices  = latitudeSegments * longitudeSegments * 6;

    // Resize vectors according to the number of vertices and indices
    mVertices.resize(vertices * 3);
    mNormals.resize(vertices * 3);
    mTextureCoordinates.resize(vertices * 2);
    mIndices.resize(indices);

    // Calculate the angle factor for latitude and longitude steps
    float latitudeFactor = glm::pi<float>() / latitudeSegments;
    float longitudeFactor = glm::two_pi<float>() / longitudeSegments;

    // Generate Vertices, Normals and Texture Coordinates
    unsigned int vertexIndex = 0;
    unsigned int textureCoordIndex = 0;
    for (unsigned int latitude = 0; latitude <= latitudeSegments; ++latitude)
    {
        float theta = latitude * latitudeFactor;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (unsigned int longitude = 0; longitude <= longitudeSegments; ++longitude)
        {
            float phi = longitude * longitudeFactor;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            // Normal Vector
            mNormals[vertexIndex]       = sinTheta * cosPhi;
            mNormals[vertexIndex + 1]   = cosTheta;
            mNormals[vertexIndex + 2]   = sinTheta * sinPhi;

            // Vertex Position
            mVertices[vertexIndex]      = radius * mNormals[vertexIndex];
            mVertices[vertexIndex + 1]  = radius * mNormals[vertexIndex + 1];
            mVertices[vertexIndex + 2]  = radius * mNormals[vertexIndex + 2];

            vertexIndex += 3;

            // Texture Coordinates
            mTextureCoordinates[textureCoordIndex]      = longitude / (float)longitudeSegments;
            mTextureCoordinates[textureCoordIndex + 1]  = latitude / (float)latitudeSegments;

            textureCoordIndex += 2;
        }
    }

    // Generate Indices
    unsigned int index = 0;
    for (unsigned int latitude = 0; latitude < latitudeSegments; ++latitude)
    {
        for (unsigned int longitude = 0; longitude < longitudeSegments; ++longitude)
        {
            // Indices calculation using clockwise winding order
            unsigned int first = (latitude * (longitudeSegments + 1)) + longitude;
            unsigned int second = first + longitudeSegments + 1;

            // First triangle
            mIndices[index] = first;
            mIndices[index + 1] = second;
            mIndices[index + 2] = first + 1;
            // Second triangle
            mIndices[index + 3] = second;
            mIndices[index + 4] = second + 1;
            mIndices[index + 5] = first + 1;

            index += 6;
        }
    }

    // Transform
    mTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
}

void Dazzle::Sphere::Draw() const
{
    if (!mVAO->IsValid())
        return;

    glBindVertexArray(mVAO->GetHandle());

    // Cull inner faces
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    // Draw
    glDrawElements(GL_TRIANGLES, (GLsizei)GetIndices().size(), GL_UNSIGNED_INT, 0);
}

void Dazzle::Sphere::Rotate(glm::vec3 axis, float degrees)
{
    auto radians = glm::radians<float>(degrees);
    mTransform = glm::rotate(mTransform, radians, axis);
}

void Dazzle::Sphere::InitializeBuffers()
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
    glVertexArrayVertexBuffer(  mVAO->GetHandle(), kNormBindingIndex, mVBO->GetHandle(), kOffset, 3 * sizeof(float));

    // Specify the format for the Normals attribute
    glVertexArrayAttribFormat(  mVAO->GetHandle(), kNormAttribIndex, kSize, kDataType, kNormalized, kOffset);
    glVertexArrayAttribBinding( mVAO->GetHandle(), kNormAttribIndex, kNormBindingIndex);
    glEnableVertexArrayAttrib(  mVAO->GetHandle(), kNormAttribIndex);

    // Bind Element Buffer Object to the element array buffer bind point of the Vertex Array Object
    glVertexArrayElementBuffer(mVAO->GetHandle(), mEBO->GetHandle());

    glBindVertexArray(mVAO->GetHandle());
}