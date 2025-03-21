#ifndef _SPHERE_HPP_
#define _SPHERE_HPP_

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
    class Sphere : public Object3D
    {
    public:
        Sphere() : Sphere(1.0f, 64, 64, 0.0f, glm::pi<float>(), 0.0f, glm::two_pi<float>()) {}
        Sphere( float radius, unsigned int latitudeSegments, unsigned int longitudeSegments,
                float thetaStart, float thetaEnd, float phiStart, float phiEnd);
        ~Sphere() override {}

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
        RenderSystem::GL::VBO* GetNVBO() const { return mNVBO.get(); }
        RenderSystem::GL::VBO* GetTCVBO() const { return mTCVBO.get(); }
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
        std::unique_ptr<RenderSystem::GL::VBO> mNVBO;   // Normal Vertex Buffer Object
        std::unique_ptr<RenderSystem::GL::VBO> mTCVBO;  // Texture Coordinates Vertex Buffer Object
        std::unique_ptr<RenderSystem::GL::EBO> mEBO;    // Elements Buffer Object
    };
}

#endif // _SPHERE_HPP_
