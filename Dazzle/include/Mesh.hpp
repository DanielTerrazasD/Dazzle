#ifndef _MESH_HPP_
#define _MESH_HPP_

#include <memory>
#include <vector>

#include "Object3D.hpp"

namespace RenderSystem::GL
{
    class VAO;
    class VBO;
    class EBO;
}

namespace Dazzle
{
    class Mesh : public Object3D
    {

    public:
        Mesh();
        ~Mesh() override {}

        void Draw() const override;
        std::vector<float> GetVertices() const override { return mVertices; }
        std::vector<float> GetNormals() const override { return mNormals; }
        std::vector<float> GetTangents() const override { return mTangents; }
        std::vector<float> GetBitangents() const override { return mBitangents; }
        std::vector<float> GetTextureCoordinates() const override { return mTextureCoordinates; }
        std::vector<unsigned int> GetIndices() const override { return std::vector<unsigned int>(); }
        glm::mat4 GetTransform() const override { return mTransform; }

        void SetName(const std::string& name) { mName = name; }
        void SetVertices(std::vector<float>&& vertices) { mVertices = std::move(vertices); }
        void SetNormals(std::vector<float>&& normals) { mNormals = std::move(normals); }
        void SetTextureCoordinates(std::vector<float>&& textureCoordinates) { mTextureCoordinates = std::move(textureCoordinates); }
        void SetTangents(std::vector<float>&& tangents) { mTangents = std::move(tangents); }

        void SetPosition(glm::vec3 position);
        void Translate(glm::vec3 position);
        void Rotate(glm::vec3 axis, float degrees);

        void InitializeBuffers();
        RenderSystem::GL::VAO* GetVAO() const { return mVAO.get(); }
        RenderSystem::GL::VBO* GetVBO() const { return mVBO.get(); }
        RenderSystem::GL::VBO* GetNVBO() const { return mNVBO.get(); }
        RenderSystem::GL::VBO* GetTCVBO() const { return mTCVBO.get(); }
        RenderSystem::GL::VBO* GetTVBO() const { return mTVBO.get(); }
        RenderSystem::GL::VBO* GetBVBO() const { return mBVBO.get(); }

    private:
        std::vector<float> mVertices;
        std::vector<float> mNormals;
        std::vector<float> mTangents;
        std::vector<float> mBitangents;
        std::vector<float> mTextureCoordinates;
        glm::mat4 mTransform;
        std::string mName;

        std::unique_ptr<RenderSystem::GL::VAO> mVAO;    // Vertex Array Object
        std::unique_ptr<RenderSystem::GL::VBO> mVBO;    // Vertex Buffer Object
        std::unique_ptr<RenderSystem::GL::VBO> mNVBO;   // Normal Vertex Buffer Object
        std::unique_ptr<RenderSystem::GL::VBO> mTCVBO;  // Texture Coordinates Vertex Buffer Object
        std::unique_ptr<RenderSystem::GL::VBO> mTVBO;   // Tangents Vertex Buffer Object
        std::unique_ptr<RenderSystem::GL::VBO> mBVBO;   // Bitangents Vertex Buffer Object
    };
}

#endif // _MESH_HPP_
