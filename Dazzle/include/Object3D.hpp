#ifndef _OBJECT_3D_HPP_
#define _OBJECT_3D_HPP_

#include <functional>
#include <vector>

#include <glm/glm.hpp>

namespace Dazzle
{
    class Object3D
    {
    public:
        virtual ~Object3D() {}

        virtual void Draw() const = 0;
        virtual std::vector<float> GetVertices() const = 0;
        virtual std::vector<float> GetNormals() const = 0;
        virtual std::vector<float> GetTangents() const = 0;
        virtual std::vector<float> GetBitangents() const = 0;
        virtual std::vector<float> GetTextureCoordinates() const = 0;
        virtual std::vector<unsigned int> GetIndices() const = 0;
        virtual glm::mat4 GetTransform() const = 0;
    };
}

#endif // _OBJECT_3D_HPP_
