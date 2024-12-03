#ifndef _SKYBOX_HPP_
#define _SKYBOX_HPP_

#include <memory>
#include <vector>

#include <glm/gtc/constants.hpp>

#include "Object3D.hpp"

namespace RenderSystem::GL
{
    class VAO;
    class VBO;
    class EBO;
}

namespace Dazzle
{
    class Skybox : public Object3D
    {
    public:
        Skybox() : Skybox(50.0f) {}
        Skybox( float length);
        ~Skybox() override {}

        void Draw() const override;
        std::vector<float> GetVertices() const override { return mVertices; }
        std::vector<float> GetNormals() const override { return mNormals; }
        std::vector<float> GetTangents() const override { return mTangents; }
        std::vector<float> GetBitangents() const override { return mBitangents; }
        std::vector<float> GetTextureCoordinates() const override { return mTextureCoordinates; }
        std::vector<unsigned int> GetIndices() const override { return mIndices; }
        glm::mat4 GetTransform() const override { return mTransform; }

        void SetPosition(glm::vec3 position);

        void Translate(glm::vec3 position);
        void Rotate(glm::vec3 axis, float degrees);
        /// TODO: void Scale(glm::vec3 factor);

        void InitializeBuffers();
        RenderSystem::GL::VAO* GetVAO() const { return mVAO.get(); }
        RenderSystem::GL::VBO* GetVBO() const { return mVBO.get(); }
        RenderSystem::GL::EBO* GetEBO() const { return mEBO.get(); }

    private:
        std::vector<float> mVertices;
        std::vector<float> mNormals;
        std::vector<float> mTangents;
        std::vector<float> mBitangents;
        std::vector<float> mTextureCoordinates;
        std::vector<unsigned int> mIndices;
        glm::mat4 mTransform;

        std::unique_ptr<RenderSystem::GL::VAO> mVAO;    // Vertex Array Object
        std::unique_ptr<RenderSystem::GL::VBO> mVBO;    // Vertex Buffer Object
        std::unique_ptr<RenderSystem::GL::EBO> mEBO;    // Elements Buffer Object
    };
}

#endif // _SKYBOX_HPP_
