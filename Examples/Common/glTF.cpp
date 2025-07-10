
#include <cstdio>
#include <cstring>

#define CGLTF_IMPLEMENTATION
#include "glTF.hpp"

void glTF::LoadGLTF(const char* path)
{
    // Load and parse file
    cgltf_options options{};
    cgltf_data* data = NULL;

    if (cgltf_parse_file(&options, path, &data) != cgltf_result_success)
    {
        std::printf("Failed to parse glTF file\n");
        return;
    }

    if (cgltf_load_buffers(&options, data, path) != cgltf_result_success)
    {
        std::printf("Failed to load glTF buffers\n");
        cgltf_free(data);
        return;
    }

    ProcessData(data);
    cgltf_free(data);
}

void glTF::ProcessData(const cgltf_data* data)
{
    if (data == nullptr)
        return;

    // File Type
    // Asset
    // Meshes
    // Materials
    // Accessors
    // Buffer Views
    // Buffers
    // Images
    // Textures
    // Samplers
    // Skins
    // Cameras
    // Lights
    // Nodes
    // Scenes
    // Animations
    // Variants
    // Data Extensions
    // Extensions Used
    // Extensions Required

    ProcessMeshes(data);
}

void glTF::ProcessNodes(const cgltf_data* data)
{
    if (data == nullptr)
        return;

    /// TODO:
}

void glTF::ProcessMeshes(const cgltf_data* data)
{
    if (data == nullptr)
        return;

    for (cgltf_size i = 0; i < data->meshes_count; ++i)
    {
        Mesh m;
        const cgltf_mesh* mesh = &data->meshes[i];
        m.mName = mesh->name ? mesh->name : "Unnamed Mesh";

        m.mPrimitives = std::move(ProcessPrimitives(mesh));
        mMeshes.push_back(std::move(m));
    }
}

std::vector<glTF::Primitive> glTF::ProcessPrimitives(const cgltf_mesh* mesh)
{
    std::vector<Primitive> primitives;
    for (cgltf_size i = 0; i < mesh->primitives_count; ++i)
    {
        Primitive p;
        const cgltf_primitive* prim = &mesh->primitives[i];

        // Set primitive type
        switch (prim->type)
        {
            case cgltf_primitive_type_points:
                p.mType = Primitive::Type::POINTS;
                break;
            case cgltf_primitive_type_lines:
                p.mType = Primitive::Type::LINES;
                break;
            case cgltf_primitive_type_line_loop:
                p.mType = Primitive::Type::LINE_LOOP;
                break;
            case cgltf_primitive_type_line_strip:
                p.mType = Primitive::Type::LINE_STRIP;
                break;
            case cgltf_primitive_type_triangles:
                p.mType = Primitive::Type::TRIANGLES;
                break;
            case cgltf_primitive_type_triangle_strip:
                p.mType = Primitive::Type::TRIANGLE_STRIP;
                break;
            case cgltf_primitive_type_triangle_fan:
                p.mType = Primitive::Type::TRIANGLE_FAN;
                break;
            default:
                p.mType = Primitive::Type::INVALID;
                std::printf("Invalid primitive type: %d\n", prim->type);
        }

        // Read vertex data
        p.FillAttributes(prim);

        // Read material
        p.FillMaterial(prim);

        primitives.push_back(std::move(p));
    }

    return primitives;
}

void glTF::Primitive::FillAttributes(const cgltf_primitive* primitive)
{
    if (primitive == nullptr)
        return;

    // Attributes accessors
    cgltf_accessor* position_accessor = nullptr;
    cgltf_accessor* normal_accessor = nullptr;
    cgltf_accessor* uv_accessor = nullptr;

    for (cgltf_size i = 0; i < primitive->attributes_count; ++i)
    {
        const cgltf_attribute* attr = &primitive->attributes[i];

        switch (attr->type)
        {
            case cgltf_attribute_type_position:
                position_accessor = attr->data;
                break;
            case cgltf_attribute_type_normal:
                normal_accessor = attr->data;
                break;
            case cgltf_attribute_type_tangent:
                // Handle tangent attribute
                break;
            case cgltf_attribute_type_texcoord:
                uv_accessor = attr->data;
                break;
            case cgltf_attribute_type_color:
                // Handle color attribute
                break;
            case cgltf_attribute_type_joints:
                // Handle joints attribute
                break;
            case cgltf_attribute_type_weights:
                // Handle weights attribute
                break;
            case cgltf_attribute_type_custom:
                // Handle custom attribute
                break;
            case cgltf_attribute_type_invalid:
            default:
                std::printf("Invalid attribute type: %d\n", attr->type);
                break;
        }
    }

    // Fill mVertices
    if (position_accessor)
    {
        // Ensure the component type is float and type is vec3
        if (position_accessor->component_type == cgltf_component_type_r_32f &&
            position_accessor->type == cgltf_type_vec3)
        {
            cgltf_size count = position_accessor->count;
            mVertices.reserve(count * 3);
            for (cgltf_size i = 0; i < count; ++i)
            {
                std::vector<float> data(3);
                cgltf_accessor_read_float(position_accessor, i, data.data(), 3);
                mVertices.insert(mVertices.end(), data.begin(), data.end());
            }
        }
    }

    // Fill mNormals
    if (normal_accessor)
    {
        // Ensure the component type is float and type is vec3
        if (normal_accessor->component_type == cgltf_component_type_r_32f &&
            normal_accessor->type == cgltf_type_vec3)
        {
            cgltf_size count = normal_accessor->count;
            mNormals.reserve(count * 3);
            for (cgltf_size i = 0; i < count; ++i)
            {
                std::vector<float> data(3);
                cgltf_accessor_read_float(normal_accessor, i, data.data(), 3);
                mNormals.insert(mNormals.end(), data.begin(), data.end());
            }
        }
    }

    // Fill mTextureCoordinates
    if (uv_accessor)
    {
        // Ensure the component type is float and type is vec2
        if (uv_accessor->component_type == cgltf_component_type_r_32f &&
            uv_accessor->type == cgltf_type_vec2)
        {
            cgltf_size count = uv_accessor->count;
            mTextureCoordinates.reserve(count * 2);
            for (cgltf_size i = 0; i < count; ++i)
            {
                std::vector<float> data(2);
                cgltf_accessor_read_float(uv_accessor, i, data.data(), 2);
                mTextureCoordinates.insert(mTextureCoordinates.end(), data.begin(), data.end());
            }
        }
    }

    // Fill mIndices
    if (primitive->indices)
    {
        const cgltf_accessor* indices_accessor = primitive->indices;
        if (indices_accessor != nullptr &&
            (indices_accessor->component_type == cgltf_component_type_r_32u ||
            indices_accessor->component_type == cgltf_component_type_r_16u))
        {
            cgltf_size count = indices_accessor->count;
            mIndices.reserve(count);
            for (cgltf_size i = 0; i < count; ++i)
            {
                size_t index = cgltf_accessor_read_index(indices_accessor, i);
                mIndices.push_back(static_cast<uint32_t>(index));
            }
        }
    }
}

void glTF::Primitive::FillMaterial(const cgltf_primitive* primitive)
{
    if (primitive == nullptr || primitive->material == nullptr)
        return;

    const cgltf_material* mat = primitive->material;

    if (mat == nullptr)
    {
        // Primitive has no material
        return;
    }

    mMaterial.mName = mat->name ? mat->name : "";
    mMaterial.mHasPBRMetallicRoughness = mat->has_pbr_metallic_roughness;
    mMaterial.mHasPBRSpecularGlossiness = mat->has_pbr_specular_glossiness;

    if (mMaterial.mHasPBRMetallicRoughness)
    {
        const cgltf_pbr_metallic_roughness* pbr = &mat->pbr_metallic_roughness;
        mMaterial.mPBRMetallicRoughness.mBaseColorFactor = {pbr->base_color_factor[0],
                                                            pbr->base_color_factor[1],
                                                            pbr->base_color_factor[2],
                                                            pbr->base_color_factor[3]};
        mMaterial.mPBRMetallicRoughness.mMetallicFactor = pbr->metallic_factor;
        mMaterial.mPBRMetallicRoughness.mRoughnessFactor = pbr->roughness_factor;

        if (pbr->base_color_texture.texture)
            mMaterial.mPBRMetallicRoughness.mBaseColorTexture = pbr->base_color_texture.texture->image->uri;

        if (pbr->metallic_roughness_texture.texture)
            mMaterial.mPBRMetallicRoughness.mMetallicRoughnessTexture = pbr->metallic_roughness_texture.texture->image->uri;
    }

    if (mMaterial.mHasPBRSpecularGlossiness)
    {
        const cgltf_pbr_specular_glossiness* pbrsg = &mat->pbr_specular_glossiness;
        mMaterial.mPBRSpecularGlossiness.mDiffuseFactor = {pbrsg->diffuse_factor[0],
                                                           pbrsg->diffuse_factor[1],
                                                           pbrsg->diffuse_factor[2],
                                                           pbrsg->diffuse_factor[3]};
        mMaterial.mPBRSpecularGlossiness.mSpecularFactor = {pbrsg->specular_factor[0],
                                                            pbrsg->specular_factor[1],
                                                            pbrsg->specular_factor[2]};
        mMaterial.mPBRSpecularGlossiness.mGlossinessFactor = pbrsg->glossiness_factor;

        if (pbrsg->diffuse_texture.texture)
            mMaterial.mPBRSpecularGlossiness.mDiffuseTexture = pbrsg->diffuse_texture.texture->image->uri;

        if (pbrsg->specular_glossiness_texture.texture)
            mMaterial.mPBRSpecularGlossiness.mSpecularGlossinessTexture = pbrsg->specular_glossiness_texture.texture->image->uri;
    }

    if (mat->normal_texture.texture)
    {
        mMaterial.mNormalTexture = mat->normal_texture.texture->image->uri;
    }

}
