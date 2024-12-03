#ifndef _UTILITIES_HPP_
#define _UTILITIES_HPP_

#include <string>
#include <vector>

namespace Utils
{
    namespace Geom
    {
        void GetMeshDataFromObj(std::string filePath,
                                std::string& name,
                                std::vector<float>& vertices,
                                std::vector<float>& normals,
                                std::vector<float>& texCoords);
        std::vector<float> GenerateNormals();
        std::vector<float> GenerateTangents(const std::vector<float>& vertices,
                                            const std::vector<float>& normals,
                                            const std::vector<float>& textureCoords);
    }

    namespace Texture
    {
        std::unique_ptr<unsigned char, void(*)(unsigned char*)> GetTextureData(const std::string& filePath, int& width, int& height, bool flip);
        std::unique_ptr<float, void(*)(float*)> GetHDRTextureData(const std::string& filePath, int& width, int& height, bool flip);
    }
}

#endif // _UTILITIES_HPP_