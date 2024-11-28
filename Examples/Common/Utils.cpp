#include <iostream>

#include "glm/glm.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Utils.hpp"

void Utils::Geom::GetMeshDataFromObj(   std::string filePath,
                                        std::string& name,
                                        std::vector<float>& vertices,
                                        std::vector<float>& normals,
                                        std::vector<float>& texCoords)
{
        tinyobj::ObjReader reader;
        if (!reader.ParseFromFile(filePath))
        {
            if (!reader.Error().empty())
                std::cerr << "Error: " << reader.Error() << std::endl;
            return;
        }

        if (!reader.Warning().empty())
            std::cout << "Warning: " << reader.Warning() << std::endl;

        // Accessing loaded data
        const tinyobj::attrib_t& attrib = reader.GetAttrib();
        const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();

        if (shapes.empty())
            return;

        // Get only the first shape.
        const auto& shape = shapes[0];
        name = shape.name;

        // Extract vertex data
        for (const auto& index : shape.mesh.indices)
        {
            // Vertex positions
            int vertex_index = 3 * index.vertex_index;
            vertices.push_back(attrib.vertices[vertex_index + 0]);
            vertices.push_back(attrib.vertices[vertex_index + 1]);
            vertices.push_back(attrib.vertices[vertex_index + 2]);

            // Normals
            if (index.normal_index >= 0)
            {
                int normal_index = 3 * index.normal_index;
                normals.push_back(attrib.normals[normal_index + 0]);
                normals.push_back(attrib.normals[normal_index + 1]);
                normals.push_back(attrib.normals[normal_index + 2]);
            }

            // Texture coordinates
            if (index.texcoord_index >= 0)
            {
                int texcoord_index = 2 * index.texcoord_index;
                texCoords.push_back(attrib.texcoords[texcoord_index + 0]);
                texCoords.push_back(attrib.texcoords[texcoord_index + 1]);
            }
        }
}

std::vector<float> Utils::Geom::GenerateNormals()
{
    return std::vector<float>();
}

std::vector<float> Utils::Geom::GenerateTangents(   const std::vector<float>& vertices,
                                                    const std::vector<float>& normals,
                                                    const std::vector<float>& textureCoords)
{
    // Return if any input is empty
    if (vertices.empty() || textureCoords.empty() || normals.empty())
        return std::vector<float>();

    size_t verticesCount = vertices.size() / 3;
    size_t normalsCount = normals.size() / 3;
    size_t textureCoordsCount = textureCoords.size() / 2;

    // Validate that inputs sizes are the same
    if (verticesCount != normalsCount || verticesCount != textureCoordsCount)
        return std::vector<float>();

    // Vectors storing the accumulated value of tangents
    std::vector<float> tangents(verticesCount * 4);
    std::vector<float> bitangents(verticesCount * 3);

    // Iterate through triangles
    for (size_t i = 0; i < verticesCount; i += 3)
    {
        float x, y, z;
        float u, v;

        // Get positions from vertices
        // Point 0 of ith triangle
        x = vertices[((i + 0) * 3 + 0)];
        y = vertices[((i + 0) * 3 + 1)];
        z = vertices[((i + 0) * 3 + 2)];
        glm::vec3 p0(x, y, z);
        
        // Point 1 of ith triangle
        x = vertices[((i + 1) * 3 + 0)];
        y = vertices[((i + 1) * 3 + 1)];
        z = vertices[((i + 1) * 3 + 2)];
        glm::vec3 p1(x, y, z);

        // Point 2 of ith triangle
        x = vertices[((i + 2) * 3 + 0)];
        y = vertices[((i + 2) * 3 + 1)];
        z = vertices[((i + 2) * 3 + 2)];
        glm::vec3 p2(x, y, z);

        // Get texture coordinates
        // Coordinate at 0
        u = textureCoords[((i + 0) * 2 + 0)];
        v = textureCoords[((i + 0) * 2 + 1)];
        glm::vec2 uv0(u, v);
        // Coordinate at 1
        u = textureCoords[((i + 1) * 2 + 0)];
        v = textureCoords[((i + 1) * 2 + 1)];
        glm::vec2 uv1(u, v);
        // Coordinate at 2
        u = textureCoords[((i + 2) * 2 + 0)];
        v = textureCoords[((i + 2) * 2 + 1)];
        glm::vec2 uv2(u, v);

        // Calculate geometry edges
        glm::vec3 edge1 = p1 - p0;
        glm::vec3 edge2 = p2 - p0;

        // Calculate texture deltas
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;

        // Calculate inverse of determinant
        float determinant = deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x;
        if (fabs(determinant) < 1e-6f)
            determinant = 1e-6f;
        float inverseDeterminant = 1.0f / determinant;

        // Calculate tangent
        // glm::vec3 tangent = inverseDeterminant * (deltaUV2.y * edge1 - deltaUV1.y * edge2); Isn't this equal?
        glm::vec3 tangent;
        tangent.x = inverseDeterminant * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = inverseDeterminant * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = inverseDeterminant * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        // Calculate bitangents
        glm::vec3 bitangent;
        bitangent.x = inverseDeterminant * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = inverseDeterminant * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = inverseDeterminant * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        // Accumulate tangents
        for (size_t j = 0; j < 3; ++j)
        {
            tangents[(i + j) * 4 + 0] += tangent.x;
            tangents[(i + j) * 4 + 1] += tangent.y;
            tangents[(i + j) * 4 + 2] += tangent.z;

            bitangents[(i + j) * 3 + 0] += bitangent.x;
            bitangents[(i + j) * 3 + 1] += bitangent.y;
            bitangents[(i + j) * 3 + 2] += bitangent.z;
        }
    }

    // Normalize tangents
    for (size_t i = 0; i < verticesCount; ++i)
    {
        glm::vec3 tangent( tangents[i * 4 + 0], tangents[i * 4 + 1], tangents[i * 4 + 2] );
        glm::vec3 normal( normals[i * 3 + 0], normals[i * 3 + 1], normals[i * 3 + 2] );

        // Gram-Schmidt orthogonalization
        tangent = glm::normalize( tangent - glm::dot(tangent, normal) * normal );

        // Calculate handedness
        // glm::vec3 bitangent = glm::cross(normal, tangent);
        glm::vec3 bitangent( bitangents[i * 3 + 0], bitangents[i * 3 + 1], bitangents[i * 3 + 2] );
        float handedness = (glm::dot(bitangent, tangent) < 0.0) ? -1.0f : 1.0f;

        tangents[i * 4 + 0] = tangent.x;
        tangents[i * 4 + 1] = tangent.y;
        tangents[i * 4 + 2] = tangent.z;
        tangents[i * 4 + 3] = handedness;
    }

    return tangents;
}