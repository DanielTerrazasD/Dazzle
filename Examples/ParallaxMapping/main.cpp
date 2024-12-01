#include <array>
#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

#include "RenderSystem.hpp"
#include "FileManager.hpp"
#include "Plane.hpp"

#include "App.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "UserInterface.hpp"
#include "Utils.hpp"

class SceneParallaxMapping : public IScene
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
        // Bump
        GLuint mBumpFactor; // (float) Bump Scale Factor for Parallax Mapping
        GLuint mSteps;      // (int) Number of steps to find the bump surface
        GLuint mMappingMode; // 0 = Parallax, 1 = Steep Parallax
    };

    SceneParallaxMapping() :  mMVP(), mModelView(), mNormalMtx(), mLocations(),
                    mKs(), mShininess(),
                    mLa(), mLds(), mLp(),
                    mBumpFactor(), mSteps(), mMappingMode() {}

    void Initialize(const std::shared_ptr<Camera>& camera) override
    {
        // Set up OpenGL debug message callback
        Dazzle::RenderSystem::GL::SetupDebugMessageCallback();

        // -----------------------------------------------------------------------------------------
        // Variables Initialization:
        mCamera = camera;

        // -----------------------------------------------------------------------------------------
        // 3D Objects for this scene:
        mPlane = std::make_unique<Dazzle::Plane>(5.0f, 5.0f, 1, 1);
        mPlane->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));
        mPlane->Rotate(glm::vec3(1.0f, 0.0f, 0.0f), 90.0f);
        mPlane->InitializeBuffers();

        // -----------------------------------------------------------------------------------------
        // Shader Program - Parallax Mapping

        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\ParallaxMapping.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\ParallaxMapping.fs.glsl");
        Dazzle::RenderSystem::GL::ShaderObject glVSO;
        Dazzle::RenderSystem::GL::ShaderObject glFSO;
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glVSO, GL_VERTEX_SHADER, VSSC);
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glFSO, GL_FRAGMENT_SHADER, FSSC);
        mProgram.Initialize();
        Dazzle::RenderSystem::GL::ProgramBuilder::Build(mProgram, {&glVSO, &glFSO});

        // Uniforms - Parallax Mapping Shader Program
        mLocations.mMVP = glGetUniformLocation(mProgram.GetHandle(), "MVP");
        mLocations.mModelView = glGetUniformLocation(mProgram.GetHandle(), "ModelView");
        mLocations.mNormal = glGetUniformLocation(mProgram.GetHandle(), "Normal");
        mLocations.mLa = glGetUniformLocation(mProgram.GetHandle(), "light.La");
        mLocations.mLds = glGetUniformLocation(mProgram.GetHandle(), "light.Lds");
        mLocations.mLp = glGetUniformLocation(mProgram.GetHandle(), "light.Lp");
        mLocations.mKs = glGetUniformLocation(mProgram.GetHandle(), "material.Ks");
        mLocations.mShininess = glGetUniformLocation(mProgram.GetHandle(), "material.Shininess");
        mLocations.mBumpFactor = glGetUniformLocation(mProgram.GetHandle(), "BumpFactor");
        mLocations.mSteps = glGetUniformLocation(mProgram.GetHandle(), "Steps");
        mLocations.mMappingMode = glGetUniformLocation(mProgram.GetHandle(), "MappingMode");

        // -----------------------------------------------------------------------------------------
        // Textures for this scene:
        GLuint bricksDiffuse = CreateTexture("textures\\brick-color.png");
        GLuint bricksNormalMap = CreateTexture("textures\\brick-normal.png");
        GLuint bricksHeightMap = CreateTexture("textures\\brick-height.png");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bricksDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bricksNormalMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, bricksHeightMap);

        // Use Program Shader
        glUseProgram(mProgram.GetHandle());
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
    }

    void Update(double time) override {}

    void Render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update Uniforms for Lights
        UpdateLight();

        // Update Uniforms for Materials
        UpdateMaterial();

        UpdateMatrices(mPlane->GetTransform());
        mPlane->Draw();
    }

    void SetKs(float Ks[3]) { mKs = glm::vec3(Ks[0], Ks[1], Ks[2]); }
    void SetShininess(float Shininess) { mShininess = Shininess; }

    void SetLa(float La[3]) { mLa = glm::vec3(La[0], La[1], La[2]); }
    void SetLds(float Lds[3]) { mLds = glm::vec3(Lds[0], Lds[1], Lds[2]); }
    void SetLp(float Lp[3]) { mLp = glm::vec4(Lp[0], Lp[1], Lp[2], 1.0f); }

    void SetBumpFactor(float bumpFactor) { mBumpFactor = bumpFactor; }
    void SetSteps(int steps) { mSteps = steps; }
    void SetMappingMode(int mode) { mMappingMode = mode; }

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

    void UpdateLight()
    {
        // Calculate Light position in View coordinates.
        glm::mat4 view = mCamera->GetTransform();
        mLp = view * mLp;

        // Update Uniforms.
        glUniform3f(mLocations.mLa, mLa.r, mLa.g, mLa.b);
        glUniform3f(mLocations.mLds, mLds.r, mLds.g, mLds.b);
        glUniform4f(mLocations.mLp, mLp.x, mLp.y, mLp.z, mLp.w);
    }

    void UpdateMaterial()
    {
        glUniform3f(mLocations.mKs, mKs.r, mKs.g, mKs.b);
        glUniform1f(mLocations.mShininess, mShininess);

        // Parallax Mapping
        glUniform1f(mLocations.mBumpFactor, mBumpFactor);
        glUniform1i(mLocations.mSteps, mSteps);
        glUniform1i(mLocations.mMappingMode, mMappingMode);
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
            glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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

    Dazzle::RenderSystem::GL::ProgramObject mProgram;
    UniformLocations mLocations;
    std::shared_ptr<Camera> mCamera;

    glm::mat4 mMVP;
    glm::mat4 mModelView;
    glm::mat3 mNormalMtx;

    std::unique_ptr<Dazzle::Plane> mPlane;

    // Light
    glm::vec3 mLa;      // Light Ambient Intensity
    glm::vec3 mLds;     // Light Diffuse & Specular Intensity
    glm::vec4 mLp;      // Light Position
    // Material
    glm::vec3 mKs;      // Material Specular Reflectivity
    float mShininess;   // Shininess Factor
    // Parallax Mapping
    float mBumpFactor;
    int mSteps;
    int mMappingMode; // 0 = Parallax, 1 = Steep Parallax
};

class UIParallaxMapping : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneParallaxMapping*>(scene); }
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

        // ImGui::ShowDemoWindow();

        ImGui::Begin("Settings");
        ImGui::Text("Press ESC to close the application.");
        ImGui::Text("Press SHIFT to toggle cursor capture mode.");

        // Shader
        ImGui::SeparatorText("Parallax Mapping");
        ImGui::Text("Mapping Mode:"); ImGui::SameLine(); ImGui::SetNextItemWidth(200.0f); ImGui::Combo("##MappingMode", &mMappingMode, "Parallax Mapping\0Steep Parallax Mapping\0\0");
        ImGui::Text("Bump Scale Factor:"); ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f); ImGui::DragFloat("##BumpFactor", &mBumpFactor, 0.0001f, 0.0f, 0.03f);
        ImGui::Text("Steps:"); ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f); ImGui::DragInt("##Steps", &mSteps, 1.0f, 60, 100);

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

            mScene->SetBumpFactor(mBumpFactor);
            mScene->SetSteps(mSteps);
            mScene->SetMappingMode(mMappingMode);
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

        // Parallax Mapping
        mBumpFactor = mBumpFactorDefault;
        mSteps = mStepsDefault;
        mMappingMode = mMappingModeDefault;
    }

    SceneParallaxMapping* mScene = nullptr;
    Camera* mCamera = nullptr;

    // Light Default Values
    const std::array<float, 3> mLaDefault = {0.12f, 0.12f, 0.12f};
    const std::array<float, 3> mLdsDefault = {0.7f, 0.7f, 0.7f};
    const std::array<float, 3> mLpDefault = {0.0f, 0.0f, -4.0f};

    // Mesh's Material Default Values
    const std::array<float, 3> mKsDefault = {0.7f, 0.7f, 0.7f};
    const float mShininessDefault = 40.0f;

    // Light
    std::array<float, 3> mLa{mLaDefault};   // Light Ambient Intensity
    std::array<float, 3> mLds{mLdsDefault}; // Light Diffuse & Specular Intensity
    std::array<float, 3> mLp{mLpDefault};   // Light Position

    // Mesh Material
    std::array<float, 3> mKs{mKsDefault};   // Material Specular Reflectivity
    float mShininess = mShininessDefault;   // Shininess Factor

    // Bump scale factor for Parallax Mapping
    const float mBumpFactorDefault = 0.03f;
    float mBumpFactor = mBumpFactorDefault;

    // Number of steps for Steep Parallax Mapping
    const int mStepsDefault = 60;
    int mSteps = mStepsDefault;

    // Selection for Parallax or Steep Parallax Mapping
    const int mMappingModeDefault = 1;
    int mMappingMode = mMappingModeDefault; // 0 = Parallax, 1 = Steep Parallax
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = 1280;    // Window Width
    config.height = 720;    // Window Height
    config.title = "Parallax Mapping";    // Window Title

    auto sceneParallaxMapping = std::make_unique<SceneParallaxMapping>();
    auto uiParallaxMapping = std::make_unique<UIParallaxMapping>();

    App app(config, std::move(sceneParallaxMapping), std::move(uiParallaxMapping));
    app.Run();

    return 0;
}
