#ifndef _CUBE_HPP_
#define _CUBE_HPP_

#include <vector>

namespace Dazzle
{
    class Cube
    {

    public:
        Cube() : Cube(1.0f) {}
        ~Cube() = default;

        Cube(float length);
        std::vector<float> GetVertices() const { return mVertices; }
        std::vector<float> GetNormals() const { return mNormals; }
        // std::vector<float> GetTangents() const { return mTangents; }
        // std::vector<float> GetBitangents() const { return mBitangents; }
        std::vector<float> GetTextureCoordinates() const { return mTextureCoordinates; }
        std::vector<unsigned int> GetIndices() const { return mIndices; }

    private:
        std::vector<float> mVertices;
        std::vector<float> mNormals;
        // std::vector<float> mTangents;
        // std::vector<float> mBitangents;
        std::vector<float> mTextureCoordinates;
        std::vector<unsigned int> mIndices;
    };
}

#endif // _CUBE_HPP_
