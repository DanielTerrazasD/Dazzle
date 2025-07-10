#ifndef _GLTF_HPP_
#define _GLTF_HPP_

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "cgltf.h"

class glTF
{
public:
    struct Material
    {
        struct PBRMetallicRoughness
        {
            std::string mBaseColorTexture; // Image file path
            std::string mMetallicRoughnessTexture; // Image file path
            std::array<float, 4> mBaseColorFactor; // RGBA
            float mMetallicFactor;
            float mRoughnessFactor;
        };

        struct PBRSpecularGlossiness
        {
            std::string mDiffuseTexture; // Image file path
            std::string mSpecularGlossinessTexture; // Image file path
            std::array<float, 4> mDiffuseFactor; // RGBA
            std::array<float, 3> mSpecularFactor; // RGB
            float mGlossinessFactor;
        };

        std::string mName;
        bool mHasPBRMetallicRoughness;
        bool mHasPBRSpecularGlossiness;
        // bool mHasClearcoat;
        // bool mHasTransmission;
        // bool mHasVolume;
        // bool mHasIOR; // Index of Refraction
        // bool mHasSpecular;
        // bool mHasSheen;
        // bool mHasEmissiveStrength;
        // bool mHasIridescence;
        // bool mHasDiffuseTransmission;
        // bool mHasAnisotropy;
        // bool mHasDispersion;

        PBRMetallicRoughness mPBRMetallicRoughness;
        PBRSpecularGlossiness mPBRSpecularGlossiness;
        std::string mNormalTexture; // Image file path
    };

    struct Primitive
    {
        Primitive() = default;
        ~Primitive() = default;

        enum class Type
        {
            INVALID = 0,
            POINTS,
            LINES,
            LINE_LOOP,
            LINE_STRIP,
            TRIANGLES,
            TRIANGLE_STRIP,
            TRIANGLE_FAN
        };

        void FillAttributes(const cgltf_primitive* primitive);
        void FillMaterial(const cgltf_primitive* primitive);

        const std::vector<uint32_t>& GetIndices() const { return mIndices; };
        const std::vector<float>& GetPositions() const { return mVertices; };
        const std::vector<float>& GetNormals() const { return mNormals; };
        const std::vector<float>& GetTextureCoordinates() const { return mTextureCoordinates; };

        Type mType = Type::INVALID; // Primitive type (e.g., triangles, lines)

        /// TODO: Define correct data type for indices
        std::vector<uint32_t> mIndices;
        std::vector<float> mVertices;
        std::vector<float> mNormals;
        // std::vector<float> mTangents;
        std::vector<float> mTextureCoordinates;
        // std::vector<float> mColor;
        // std::vector<float> mJoints;
        // std::vector<float> mWeights;
        // std::vector<float> mCustom;

        Material mMaterial;
    };

    struct Mesh
    {
        std::string mName;
        std::vector<Primitive> mPrimitives;
    };

    glTF() = default;
    ~glTF() = default;

    void LoadGLTF(const char* path);

    std::vector<Mesh> GetMeshes() const { return mMeshes; }

private:

    void ProcessData(const cgltf_data* data);
    void ProcessNodes(const cgltf_data* data);
    void ProcessMeshes(const cgltf_data* data);

    std::vector<Primitive> ProcessPrimitives(const cgltf_mesh* mesh);

    std::vector<Mesh> mMeshes;
    size_t mMeshCount = 0;
    size_t mMaterialCount = 0;
    size_t mImagesCount = 0;
    size_t mTexturesCount = 0;
    size_t mLightsCount = 0;
    size_t mNodesCount = 0;
    size_t mScenesCount = 0;
};

#endif // _GLTF_HPP_