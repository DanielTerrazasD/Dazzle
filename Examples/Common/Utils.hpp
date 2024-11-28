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
}

#endif // _UTILITIES_HPP_