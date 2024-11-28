#include <cstdio>
#include <array>
#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "RenderSystem.hpp"
#include "FileManager.hpp"
#include "Mesh.hpp"

#include "App.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "UserInterface.hpp"
#include "Utils.hpp"

class SceneNormalMapping : public IScene
{
public:
    struct UniformLocations
    {
        GLuint mMVP;        // (mat4) Model View Projection
        GLuint mModelView;  // (mat4) Model View
        GLuint mNormal;     // (mat3) Normal

        // Light
        GLuint mLa;         // (vec3) Ambien Intensity
        GLuint mLds;         // (vec3) Diffuse Intensity
        GLuint mLp;         // (vec4) Light Position In View Coordinates

        // Material
        GLuint mKs;         // (vec3) Specular Reflectivity
        GLuint mShininess;  // (float) Shininess
    };

    SceneNormalMapping() :  mMVP(), mModelView(), mNormalMtx(), mLocations(),
                    mKs(), mShininess(),
                    mLa(), mLds(), mLp() {}

    void Initialize(const std::shared_ptr<Camera>& camera) override
    {
        // Set up OpenGL debug message callback
        Dazzle::RenderSystem::GL::SetupDebugMessageCallback();

        // -----------------------------------------------------------------------------------------
        // Variables Initialization:
        mCamera = camera;

        // -----------------------------------------------------------------------------------------
        // 3D Objects for this scene:
        mOgre = std::make_unique<Dazzle::Mesh>();
        mOgre->SetPosition(glm::vec3(0.0f, 0.0f, -3.0f));
        InitializeMesh(*mOgre, "models\\bs_ears.obj");
        mOgre->InitializeBuffers();

        // -----------------------------------------------------------------------------------------
        // Shader Program
        // Get the source code for shaders
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\NormalMapping.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\NormalMapping.fs.glsl");

        // Create OpenGL shader objects and build them using the respecting source code.
        Dazzle::RenderSystem::GL::ShaderObject glVSO;
        Dazzle::RenderSystem::GL::ShaderObject glFSO;
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glVSO, GL_VERTEX_SHADER, VSSC);
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glFSO, GL_FRAGMENT_SHADER, FSSC);

        // Create and build the OpenGL program object.
        mProgram.Initialize();
        Dazzle::RenderSystem::GL::ProgramBuilder::Build(mProgram, {&glVSO, &glFSO});

        // -----------------------------------------------------------------------------------------
        // Textures for this scene:
        GLuint ogreDiffuse = CreateTexture("textures\\diffuse.png", true);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ogreDiffuse);
        GLuint ogreNormalMap = CreateTexture("textures\\normalmap.png", true);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ogreNormalMap);

        // Get Uniforms
        mLocations.mMVP = glGetUniformLocation(mProgram.GetHandle(), "MVP");                        // (mat4) Model View Projection
        mLocations.mModelView = glGetUniformLocation(mProgram.GetHandle(), "ModelView");            // (mat4) Model View
        mLocations.mNormal = glGetUniformLocation(mProgram.GetHandle(), "Normal");                  // (mat3) Normal
        // Uniforms - Light
        mLocations.mLa = glGetUniformLocation(mProgram.GetHandle(), "light.La");                    // (vec3) Ambient Intensity
        mLocations.mLds = glGetUniformLocation(mProgram.GetHandle(), "light.Lds");                    // (vec3) Diffuse Intensity
        mLocations.mLp = glGetUniformLocation(mProgram.GetHandle(), "light.Lp");                    // (vec4) Position In View Coordinates
        // Uniforms - Material
        mLocations.mKs = glGetUniformLocation(mProgram.GetHandle(), "material.Ks");                 // (vec3) Specular Intensity
        mLocations.mShininess = glGetUniformLocation(mProgram.GetHandle(), "material.Shininess");   // (float) Shininess Factor

        // Use Program Shader
        glUseProgram(mProgram.GetHandle());
        glEnable(GL_DEPTH_TEST);
    }

    void Update(double time) override {}

    void Render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update Uniforms for Lights
        UpdateLights();

        // Update Uniforms for Mesh.
        glUniform3f(mLocations.mKs, mKs.r, mKs.g, mKs.b);
        glUniform1f(mLocations.mShininess, mShininess);

        UpdateMatrices(mOgre->GetTransform());
        mOgre->Draw();
    }

    void SetKs(float Ks[3]) { mKs = glm::vec3(Ks[0], Ks[1], Ks[2]); }
    void SetShininess(float Shininess) { mShininess = Shininess; }

    void SetLa(float La[3]) { mLa = glm::vec3(La[0], La[1], La[2]); }
    void SetLds(float Lds[3]) { mLds = glm::vec3(Lds[0], Lds[1], Lds[2]); }
    void SetLp(float Lp[3]) { mLp = glm::vec4(Lp[0], Lp[1], Lp[2], 1.0f); }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override { glViewport(0, 0, width, height); }

private:
    void UpdateMatrices(glm::mat4 model)
    {
        glm::mat4 view = mCamera->GetTransform();
        glm::mat4 projection = mCamera->GetProjection();

        mModelView = view * model;
        mMVP = projection * mModelView;
        mNormalMtx = glm::transpose(glm::inverse(mModelView));

        glUniformMatrix4fv(mLocations.mMVP, 1, GL_FALSE, glm::value_ptr(mMVP));
        glUniformMatrix4fv(mLocations.mModelView, 1, GL_FALSE, glm::value_ptr(mModelView));
        glUniformMatrix3fv(mLocations.mNormal, 1, GL_FALSE, glm::value_ptr(mNormalMtx));
    }

    void UpdateLights()
    {
        // Calculate Light position in View coordinates.
        glm::mat4 view = mCamera->GetTransform();
        mLp = view * mLp;

        // Update Uniforms.
        glUniform3f(mLocations.mLa, mLa.r, mLa.g, mLa.b);
        glUniform3f(mLocations.mLds, mLds.r, mLds.g, mLds.b);
        glUniform4f(mLocations.mLp, mLp.x, mLp.y, mLp.z, mLp.w);
    }

    GLuint CreateTexture(const std::string& path, bool flipXY)
    {
        int width, height;
        unsigned char* data = GetTextureData(path, width, height, flipXY);
        GLuint texture = 0; // OpenGL Texture Object
        if (data != nullptr)
        {
            // Create texture
            glCreateTextures(GL_TEXTURE_2D, 1, &texture);

            // Set texture parameters
            glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            // Explicitly state the base and max levels
            glTextureParameteri(texture, GL_TEXTURE_BASE_LEVEL, 0);
            glTextureParameteri(texture, GL_TEXTURE_MAX_LEVEL, 0);

            // Set up texture storage
            glTextureStorage2D(texture, 1, GL_RGBA8, width, height);

            // Upload texture data
            glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

            // Free image data
            stbi_image_free(data);
        }

        return texture;
    }

    unsigned char* GetTextureData(const std::string& path, int& width, int& height, bool flip)
    {
        int bytesPerPixel;
        const int desiredChannels = 4;
        stbi_set_flip_vertically_on_load(flip);
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &bytesPerPixel, desiredChannels);
        return data;
    }

    void InitializeMesh(Dazzle::Mesh& mesh, const std::string& path)
    {
        std::string name;
        std::vector<float> vertices, normals, textureCoords;
        Utils::Geom::GetMeshDataFromObj(path, name, vertices, normals, textureCoords);
        mesh.SetName(name);
        mesh.SetVertices(std::move(vertices));
        mesh.SetNormals(std::move(normals));
        mesh.SetTextureCoordinates(std::move(textureCoords));
        mesh.SetTangents( Utils::Geom::GenerateTangents(mesh.GetVertices(),
                                                        mesh.GetNormals(),
                                                        mesh.GetTextureCoordinates()));
    }

    Dazzle::RenderSystem::GL::ProgramObject mProgram;
    std::shared_ptr<Camera> mCamera;
    UniformLocations mLocations;

    glm::mat4 mMVP;
    glm::mat4 mModelView;
    glm::mat3 mNormalMtx;

    std::unique_ptr<Dazzle::Mesh> mOgre;

    // Light
    glm::vec3 mLa;      // Light Ambient Intensity
    glm::vec3 mLds;     // Light Diffuse & Specular Intensity
    glm::vec4 mLp;      // Light Position
    // Material
    glm::vec3 mKs;      // Material Specular Reflectivity
    float mShininess;   // Shininess Factor
};

class UINormalMapping : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneNormalMapping*>(scene); }
    void SetCamera(Camera* camera) override { mCamera = camera; }

    void Update() override
    {
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

        ImGui::Begin("Settings");
        ImGui::Text("Press ESC to close the application.");
        ImGui::Text("Press SHIFT to toggle cursor capture mode.");

        // Shader
        ImGui::SeparatorText("Normal Mapping");

        // Variables
        ImGui::NewLine();
        ImGui::Text("Variables:");
        ImGui::Text("Lp: Light Position");
        ImGui::Text("La: Light Ambient Intensity");
        ImGui::Text("Lds: Light Diffuse & Specular Intensity");
        ImGui::Text("Ks: Material Specular Reflectivity");

        ImGui::NewLine();
        ImGui::Text("Positional Light:");
        ImGui::Text("Lp:"); ImGui::SameLine(); ImGui::DragFloat3("##Light Position", mLp.data(), 0.1f, -10.0f, 10.0f);
        ImGui::Text("La:"); ImGui::SameLine(); ImGui::ColorEdit3("##Ambient Intensity", mLa.data());
        ImGui::Text("Lds:"); ImGui::SameLine(); ImGui::ColorEdit3("##Diffuse Intensity", mLds.data());
        // Mesh Material
        ImGui::NewLine();
        ImGui::Text("Mesh:");
        ImGui::Text("Ks:"); ImGui::SameLine(); ImGui::ColorEdit3("##Specular Reflectivity", mKs.data());
        ImGui::Text("Shininess:"); ImGui::SameLine(); ImGui::PushItemWidth(100.0f); ImGui::DragFloat("##Shininess", &mShininess, 1.0f, 1.0f, 300.0f); ImGui::PopItemWidth();

        if (ImGui::Button("Reset"))
            ResetToDefaults();

        // Camera
        ImGui::SeparatorText("Camera");
        ImGui::Text("Position - X: %.2f, Y: %.2f, Z: %.2f", cameraPosition.x, cameraPosition.y, cameraPosition.z);
        ImGui::Text("Yaw: %.2f, Pitch: %.2f", cameraYaw, cameraPitch);
        ImGui::End();

        // Set data to the scene
        if (mScene)
        {
            // Mesh
            mScene->SetKs(mKs.data());
            mScene->SetShininess(mShininess);

            // Light
            mScene->SetLa(mLa.data());
            mScene->SetLds(mLds.data());
            mScene->SetLp(mLp.data());
        }
    }

private:
    void ResetToDefaults()
    {
        // Mesh
        mKs = mKsDefault;
        mShininess = mShininessDefault;

        // Light
        mLa = mLaDefault;
        mLds = mLdsDefault;
        mLp = mLpDefault;
    }

    SceneNormalMapping* mScene = nullptr;
    Camera* mCamera = nullptr;

    // Light Default Values
    const std::array<float, 3> mLaDefault = {0.15f, 0.15f, 0.15f};
    const std::array<float, 3> mLdsDefault = {1.0f, 1.0f, 1.0f};
    const std::array<float, 3> mLpDefault = {3.0f, 3.0f, -2.0f};

    // Mesh's Material Default Values
    const std::array<float, 3> mKsDefault = {0.05f, 0.05f, 0.05f};
    const float mShininessDefault = 1.0f;

    // Light
    std::array<float, 3> mLa{mLaDefault};   // Light Ambient Intensity
    std::array<float, 3> mLds{mLdsDefault}; // Light Diffuse & Specular Intensity
    std::array<float, 3> mLp{mLpDefault};   // Light Position

    // Mesh Material
    std::array<float, 3> mKs{mKsDefault};   // Material Specular Reflectivity
    float mShininess = mShininessDefault;   // Shininess Factor
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = 1280;    // Window Width
    config.height = 720;    // Window Height
    config.title = "Normal Mapping";    // Window Title

    auto sceneNormalMapping = std::make_unique<SceneNormalMapping>();
    auto uiNormalMapping = std::make_unique<UINormalMapping>();

    App app(config, std::move(sceneNormalMapping), std::move(uiNormalMapping));
    app.Run();

    return 0;
}
