#include <array>
#include <iostream>
#include <unordered_map>

// #define CGLTF_IMPLEMENTATION
// #include "cgltf.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"

#include "RenderSystem.hpp"
#include "FileManager.hpp"

#include "App.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "UserInterface.hpp"
#include "glTF.hpp"
#include "Utils.hpp"
#include "Mesh.hpp"

class SceneglTF : public IScene
{
public:

    struct ShaderProgram
    {
        std::unordered_map<std::string, GLuint> mLocations;
        Dazzle::RenderSystem::GL::ProgramObject mProgram;
    };

    struct Light
    {
        glm::vec4 mPosition;
        glm::vec3 mIntensity;
    };

    SceneglTF() :  mMVP() {}

    void Initialize(const std::shared_ptr<Camera>& camera) override
    {
        // Set up OpenGL debug message callback
        Dazzle::RenderSystem::GL::SetupDebugMessageCallback();

        // -----------------------------------------------------------------------------------------
        // Variables Initialization:
        mCamera = camera;

        // -----------------------------------------------------------------------------------------
        // 3D Objects used by this scene:
        mWoodenTable.LoadGLTF("models/round_wooden_table_02_4k.gltf/round_wooden_table_02_4k.gltf");

        auto meshes = mWoodenTable.GetMeshes();
        auto primitive = meshes[0].mPrimitives[0];
        mWoodenTableVAO = InitializeBuffers(primitive.GetPositions(),
                                            primitive.GetNormals(),
                                            primitive.GetTextureCoordinates(),
                                            Utils::Geom::GenerateTangents(primitive.GetPositions(), primitive.GetNormals(), primitive.GetTextureCoordinates()),
                                            primitive.GetIndices());

        // Create Textures
        mAlbedoTexture = CreateTexture("models/round_wooden_table_02_4k.gltf/" + 
                                       primitive.mMaterial.mPBRMetallicRoughness.mBaseColorTexture, false);
        mARMTexture = CreateTexture("models/round_wooden_table_02_4k.gltf/" + 
                                       primitive.mMaterial.mPBRMetallicRoughness.mMetallicRoughnessTexture, false);
        mNormalTexture = CreateTexture("models/round_wooden_table_02_4k.gltf/" + 
                                       primitive.mMaterial.mNormalTexture, false);

        glBindTextureUnit(0, mAlbedoTexture);
        glBindTextureUnit(1, mNormalTexture);
        glBindTextureUnit(2, mARMTexture);

        // -----------------------------------------------------------------------------------------
        // Shader Program
        InitializeShaderProgram();
        InitializeUniforms();
    }

    void Update(double time) override {}

    void Render() override
    {
        DrawScene();
    }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override { glViewport(0, 0, width, height); }

    void SetLightPosition(const std::array<float, 3> pos) { mLightPosition = glm::vec3(pos[0], pos[1], pos[2]); }

private:

    void DrawScene()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        UpdateMatrices(mShader, glm::mat4(1.0f));
        UpdateUniforms();
        glBindVertexArray(mWoodenTableVAO);
        glDrawElements(GL_TRIANGLES, 12762, GL_UNSIGNED_INT, nullptr);
    }

    void InitializeShaderProgram()
    {
        // Get the source code for shaders
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\glTF.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\glTF.fs.glsl");

        // Create OpenGL shader objects and build them using the respecting source code.
        Dazzle::RenderSystem::GL::ShaderObject glVSO;
        Dazzle::RenderSystem::GL::ShaderObject glFSO;
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glVSO, GL_VERTEX_SHADER, VSSC);
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glFSO, GL_FRAGMENT_SHADER, FSSC);

        // Create and build the OpenGL program object.
        mShader.mProgram.Initialize();
        Dazzle::RenderSystem::GL::ProgramBuilder::Build(mShader.mProgram, {&glVSO, &glFSO});

        // Get Uniforms
        GLuint handle = mShader.mProgram.GetHandle();
        mShader.mLocations["uModelMatrix"] = glGetUniformLocation(handle, "uModelMatrix");
        mShader.mLocations["uViewMatrix"] = glGetUniformLocation(handle, "uViewMatrix");
        mShader.mLocations["uProjectionMatrix"] = glGetUniformLocation(handle, "uProjectionMatrix");
        mShader.mLocations["uNormalMatrix"] = glGetUniformLocation(handle, "uNormalMatrix");

        mShader.mLocations["uCameraPosition"] = glGetUniformLocation(handle, "uCameraPosition");
        mShader.mLocations["uLightPosition"] = glGetUniformLocation(handle, "uLightPosition");
        mShader.mLocations["uLightColor"] = glGetUniformLocation(handle, "uLightColor");

        glUseProgram(mShader.mProgram.GetHandle());
        glEnable(GL_DEPTH_TEST);
        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_BACK);
    }

    void InitializeUniforms()
    {
    }

    GLuint InitializeBuffers(   const std::vector<float>& positions,
                                const std::vector<float>& normals,
                                const std::vector<float>& uvs,
                                const std::vector<float>& tangents,
                                const std::vector<uint32_t>& indices)
    {
        const GLintptr kOffset = 0;
        const GLintptr kStride = 0;
        const GLuint kSize = 3; // The number of values per vertex that are stored in the array.
        const GLuint kTexSize = 2; // Number of values per vertex texture coordinate
        const GLenum kDataType = GL_FLOAT;
        const GLboolean kNormalized = GL_FALSE;

        // Vertices
        const GLuint kPosAttribIndex = 0;
        const GLuint kPosBindingIndex = 0;
        // Normals
        const GLuint kNormAttribIndex = 1;
        const GLuint kNormBindingIndex = 1;
        // Texture Coordinates
        const GLuint kTexAttribIndex = 2;
        const GLuint kTexBindingIndex = 2;
        // Tangents
        const GLuint kTangAttribIndex = 3;
        const GLuint kTangBindingIndex = 3;
        // Bitangents
        const GLuint kBitanAttribIndex = 4;
        const GLuint kBitanBindingIndex = 4;

        GLuint VAO, VBO, NVBO, TCVBO, TVBO, EBO;
        glCreateVertexArrays(1, &VAO);

        glCreateBuffers(1, &VBO);
        glCreateBuffers(1, &NVBO);
        glCreateBuffers(1, &TCVBO);
        glCreateBuffers(1, &TVBO);
        glCreateBuffers(1, &EBO);

        // Set up the data store for the Vertex Buffer Object for Vertices
        glNamedBufferStorage(VBO, positions.size() * sizeof(float), positions.data(), 0);

        // Set up the data store for the Vertex Buffer Object for Normals
        glNamedBufferStorage(NVBO, normals.size() * sizeof(float), normals.data(), 0);

        // Set up the data store for the Vertex Buffer Object for Texture Coordinates
        glNamedBufferStorage(TCVBO, uvs.size() * sizeof(float), uvs.data(), 0);

        // Set up the data store for the Vertex Buffer Object for Tangents
        glNamedBufferStorage(TVBO, tangents.size() * sizeof(float), tangents.data(), 0);

        // Set up the data store for the Element Buffer Object for Indices
        glNamedBufferStorage(EBO, indices.size() * sizeof(uint32_t), indices.data(), 0);

        // VBO
        glVertexArrayVertexBuffer(VAO, kPosBindingIndex, VBO, kOffset, 3 * sizeof(float));
        glVertexArrayAttribFormat(VAO, kPosAttribIndex, kSize, kDataType, kNormalized, kOffset);
        glVertexArrayAttribBinding(VAO, kPosAttribIndex, kPosBindingIndex);
        glEnableVertexArrayAttrib(VAO, kPosAttribIndex);

        // Normals
        glVertexArrayVertexBuffer(VAO, kNormBindingIndex, NVBO, kOffset, 3 * sizeof(float));
        glVertexArrayAttribFormat(VAO, kNormAttribIndex, kSize, kDataType, kNormalized, kOffset);
        glVertexArrayAttribBinding(VAO, kNormAttribIndex, kNormBindingIndex);
        glEnableVertexArrayAttrib(VAO, kNormAttribIndex);

        // Texture Coordinates
        glVertexArrayVertexBuffer(VAO, kTexBindingIndex, TCVBO, kOffset, 2 * sizeof(float));
        glVertexArrayAttribFormat(VAO, kTexAttribIndex, kTexSize, kDataType, kNormalized, kOffset);
        glVertexArrayAttribBinding(VAO, kTexAttribIndex, kTexBindingIndex);
        glEnableVertexArrayAttrib(VAO, kTexAttribIndex);

        // Tangents
        glVertexArrayVertexBuffer(VAO, kTangBindingIndex, TVBO, kOffset, 4 * sizeof(float));
        glVertexArrayAttribFormat(VAO, kTangAttribIndex, kSize, kDataType, kNormalized, kOffset);
        glVertexArrayAttribBinding(VAO, kTangAttribIndex, kTangBindingIndex);
        glEnableVertexArrayAttrib(VAO, kTangAttribIndex);

        // EBO
        glVertexArrayElementBuffer(VAO, EBO);

        glBindVertexArray(VAO);
        return VAO;
    }

    void UpdateMatrices(const ShaderProgram& shader, glm::mat4 model)
    {
        auto view = mCamera->GetTransform();
        auto projection = mCamera->GetProjection();
        auto normal = glm::transpose(glm::inverse(glm::mat3(model)));

        auto location = shader.mLocations.find("uModelMatrix");
        if (location != shader.mLocations.end())
           glUniformMatrix4fv(location->second, 1, GL_FALSE, glm::value_ptr(model));

        location = shader.mLocations.find("uViewMatrix");
        if (location != shader.mLocations.end())
           glUniformMatrix4fv(location->second, 1, GL_FALSE, glm::value_ptr(view));

        location = shader.mLocations.find("uProjectionMatrix");
        if (location != shader.mLocations.end())
           glUniformMatrix4fv(location->second, 1, GL_FALSE, glm::value_ptr(projection));

        location = shader.mLocations.find("uNormalMatrix");
        if (location != shader.mLocations.end())
           glUniformMatrix3fv(location->second, 1, GL_FALSE, glm::value_ptr(normal));
    }

    void UpdateUniforms()
    {
        glm::vec3 cameraPosition = mCamera->GetPosition();
        glUniform3fv(mShader.mLocations.at("uCameraPosition"), 1, glm::value_ptr(cameraPosition));
        glUniform3fv(mShader.mLocations.at("uLightPosition"), 1, glm::value_ptr(mLightPosition));
        glUniform3fv(mShader.mLocations.at("uLightColor"), 1, glm::value_ptr(mLightColor));
    }

    GLuint CreateTexture(const std::string& path, bool flip = true)
    {
        int width, height;
        auto imageData = Utils::Texture::GetTextureData(path, width, height, flip);
        GLuint texture = 0; // OpenGL Texture Object
        if (imageData)
        {
            // Create texture
            glCreateTextures(GL_TEXTURE_2D, 1, &texture);

            // Set texture parameters
            glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            // Explicitly state the base and max levels
            glTextureParameteri(texture, GL_TEXTURE_BASE_LEVEL, 0);
            glTextureParameteri(texture, GL_TEXTURE_MAX_LEVEL, 0);

            // Set up texture storage
            glTextureStorage2D(texture, 1, GL_RGBA8, width, height);

            // Upload texture data
            glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, imageData.get());
        }

        return texture;
    }

    glTF mWoodenTable;
    ShaderProgram mShader;
    glm::vec3 mLightPosition = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 mLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    std::shared_ptr<Camera> mCamera;
    glm::mat4 mMVP;

    GLuint mAlbedoTexture;
    GLuint mNormalTexture;
    GLuint mARMTexture;

    GLuint mWoodenTableVAO;
};

class UIglTF : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneglTF*>(scene); }
    void SetCamera(Camera* camera) override { mCamera = camera; }

    void Update() override
    {
        // Get data from the scene
        if (mScene)
        {
        }

        // Get data from the camera
        glm::vec3 cameraPosition = glm::vec3();
        float cameraYaw = 0.0f;
        float cameraPitch = 0.0f;
        if (mCamera)
        {
            cameraPosition = mCamera->GetPosition();
            cameraYaw = mCamera->GetYaw();
            cameraPitch = mCamera->GetPitch();
        }

        static float column = 130.0f;
        static float comboWidth = 160.0f;

        ImGui::Begin("Settings");
        ImGui::Text("Press ESC to close the application.");
        ImGui::Text("Press SHIFT to toggle cursor capture mode.");

        // Shader
        ImGui::SeparatorText("");
        ImGui::Text("Light Position:"); ImGui::SameLine();
        ImGui::DragFloat3("##LightPosition", mLightPosition.data(), 0.01f, -10.0f, 10.0f);

        // Camera
        ImGui::SeparatorText("Camera");
        ImGui::Text("Position - X: %.2f, Y: %.2f, Z: %.2f", cameraPosition.x, cameraPosition.y, cameraPosition.z);
        ImGui::Text("Yaw: %.2f, Pitch: %.2f", cameraYaw, cameraPitch);
        ImGui::End();

        // Set data to the scene
        if (mScene)
        {
            mScene->SetLightPosition(mLightPosition);
        }
    }

private:
    SceneglTF* mScene = nullptr;
    Camera* mCamera = nullptr;

    std::array<float, 3> mLightPosition = { 1.0f, 1.0f, 1.0f };
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = 1280;        // Window Width
    config.height = 720;        // Window Height
    config.title = "glTF + PBR + NormalMapping";   // Window Title

    auto sceneglTF = std::make_unique<SceneglTF>();
    auto uiglTF = std::make_unique<UIglTF>();

    App app(config, std::move(sceneglTF), std::move(uiglTF));
    app.Run();

    return 0;
}
