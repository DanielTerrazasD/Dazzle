#ifndef _CUBE_HPP_
#define _CUBE_HPP_

#include <memory>
#include <vector>

#include "Object3D.hpp"
#include "RenderSystem.hpp"

namespace Dazzle
{
    class Cube : public Object3D
    {

    public:
        Cube() : Cube(1.0f) {}
        Cube(float length);
        ~Cube() override {}

        void Draw() const override;
        std::vector<float> GetVertices() const override { return mVertices; }
        std::vector<float> GetNormals() const override { return mNormals; }
        std::vector<float> GetTangents() const override { return mTangents; }
        std::vector<float> GetBitangents() const override { return mBitangents; }
        std::vector<float> GetTextureCoordinates() const override { return mTextureCoordinates; }
        std::vector<unsigned int> GetIndices() const override { return mIndices; }
        glm::mat4 GetTransform() const override { return mTransform; }


        /// TODO: void Tranlate(glm::vec3 position);
        void Rotate(glm::vec3 axis, float degrees);
        /// TODO: void Scale(glm::vec3 factor);

        void SetUpBuffers(GLuint bindingIndex, GLuint attributeIndex);
        RenderSystem::GL::VAO GetVAO() const { return mVAO; }
        RenderSystem::GL::VBO GetVBO() const { return mVBO; }
        RenderSystem::GL::EBO GetEBO() const { return mEBO; }

    private:
        std::vector<float> mVertices;
        std::vector<float> mNormals;
        std::vector<float> mTangents;
        std::vector<float> mBitangents;
        std::vector<float> mTextureCoordinates;
        std::vector<unsigned int> mIndices;
        glm::mat4 mTransform;

        bool mAreBuffersSet;
        RenderSystem::GL::VAO mVAO;
        RenderSystem::GL::VBO mVBO;
        RenderSystem::GL::EBO mEBO;
    };
}

#endif // _CUBE_HPP_
