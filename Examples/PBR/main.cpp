#include <array>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"

#include "RenderSystem.hpp"
#include "FileManager.hpp"
#include "Plane.hpp"
#include "Sphere.hpp"
#include "Torus.hpp"

#include "App.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "UserInterface.hpp"

class ScenePBR : public IScene
{
public:
    struct LightUniforms
    {
        GLuint mPosition;   // (vec4) Light Position In View Coordinates
        GLuint mIntensity;  // (vec3) Intensity
    };

    struct UniformLocations
    {
        GLuint mMVP;        // (mat4) Model View Projection
        GLuint mModelView;  // (mat4) Model View
        GLuint mNormal;     // (mat3) Normal

        // Lights
        LightUniforms mLights[3];

        // Material
        GLuint mRoughness;     // (float) Material's Roughness
        GLuint mMetal;         // (bool) Metal or Dielectric
        GLuint mColor;         // (vec3) Material's Color
    };


    ScenePBR() :    mMVP(), mModelView(), mNormalMtx(), mLocations(),
                    mLights(), mTorusMaterial() {}

    void Initialize(const std::shared_ptr<Camera>& camera) override
    {
        // Set up OpenGL debug message callback
        Dazzle::RenderSystem::GL::SetupDebugMessageCallback();

        // -----------------------------------------------------------------------------------------
        // Variables Initialization:
        mCamera = camera;

        // -----------------------------------------------------------------------------------------
        // 3D Objects for this scene:
        mPlane = std::make_unique<Dazzle::Plane>(); 
        mPlane->SetPosition(glm::vec3(0.0f, -1.0f, 0.0f));
        mPlane->InitializeBuffers();

        mSphere = std::make_unique<Dazzle::Sphere>(0.5f, 128, 128, 0.0f, glm::pi<float>(), 0.0f, glm::two_pi<float>());
        mSphere->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        mSphere->InitializeBuffers();

        mTorus = std::make_unique<Dazzle::Torus>(0.5f, 0.25f, 128, 128, 0.0f, glm::two_pi<float>(), 0.0f, glm::two_pi<float>());
        mTorus->SetPosition(glm::vec3(0.0f, 0.0f, 1.0f));
        mTorus->InitializeBuffers();

        // -----------------------------------------------------------------------------------------
        // Shader Program
        // Get the source code for shaders
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\PBR.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\PBR.fs.glsl");

        // Create OpenGL shader objects and build them using the respecting source code.
        Dazzle::RenderSystem::GL::ShaderObject glVSO;
        Dazzle::RenderSystem::GL::ShaderObject glFSO;
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glVSO, GL_VERTEX_SHADER, VSSC);
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glFSO, GL_FRAGMENT_SHADER, FSSC);

        // Create and build the OpenGL program object.
        mProgram.Initialize();
        Dazzle::RenderSystem::GL::ProgramBuilder::Build(mProgram, {&glVSO, &glFSO});

        // Get Uniforms
        mLocations.mMVP = glGetUniformLocation(mProgram.GetHandle(), "MVP");
        mLocations.mModelView = glGetUniformLocation(mProgram.GetHandle(), "ModelView");
        mLocations.mNormal = glGetUniformLocation(mProgram.GetHandle(), "Normal");
        // Uniforms - Lights
        mLocations.mLights[0].mPosition = glGetUniformLocation(mProgram.GetHandle(), "light[0].Position");
        mLocations.mLights[0].mIntensity = glGetUniformLocation(mProgram.GetHandle(), "light[0].Intensity");
        mLocations.mLights[1].mPosition = glGetUniformLocation(mProgram.GetHandle(), "light[1].Position");
        mLocations.mLights[1].mIntensity = glGetUniformLocation(mProgram.GetHandle(), "light[1].Intensity");
        mLocations.mLights[2].mPosition = glGetUniformLocation(mProgram.GetHandle(), "light[2].Position");
        mLocations.mLights[2].mIntensity = glGetUniformLocation(mProgram.GetHandle(), "light[2].Intensity");
        // Uniforms - Material
        mLocations.mRoughness = glGetUniformLocation(mProgram.GetHandle(), "material.Roughness");
        mLocations.mMetal = glGetUniformLocation(mProgram.GetHandle(), "material.Metal");
        mLocations.mColor = glGetUniformLocation(mProgram.GetHandle(), "material.Color");

        // Use Program Shader
        glUseProgram(mProgram.GetHandle());
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    void Update(double time) override {}

    void Render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model;

        // Lights
        UpdateLights();

        // Plane
        const glm::vec3 planeColor(0.2f);
        const float planeRoughness = 0.9f;
        UpdateMaterial(planeRoughness, false, planeColor);
        UpdateMatrices(mPlane->GetTransform());
        mPlane->Draw();

        // Dielectric Objects
        const bool isMetal = false;
        const unsigned int numObjects = 9;
        const glm::vec3 baseColor(0.1f, 0.33f, 0.17f);
        for (unsigned int i = 0; i < numObjects; i++)
        {
            float roughness = (i + 1) * (1.0f / numObjects);
            UpdateMaterial(roughness, isMetal, baseColor);

            float deltaX = i * (10.0f / (numObjects - 1)) - 5.0f;
            model = mSphere->GetTransform();
            model = glm::translate(model, glm::vec3(deltaX, 2.0f, -2.0f));
            UpdateMatrices(model);

            mSphere->Draw();
        }

        // Metallic Objects
        model = mSphere->GetTransform();
        UpdateMaterial(kGold);
        UpdateMatrices(glm::translate(model, glm::vec3(-3.0f, 1.0f, -1.0f)));
        mSphere->Draw();

        UpdateMaterial(kCopper);
        UpdateMatrices(glm::translate(model, glm::vec3(-1.5f, 1.0f, -1.0f)));
        mSphere->Draw();

        UpdateMaterial(kAluminum);
        UpdateMatrices(glm::translate(model, glm::vec3(0.0f, 1.0f, -1.0f)));
        mSphere->Draw();

        UpdateMaterial(kTitanium);
        UpdateMatrices(glm::translate(model, glm::vec3(1.5f, 1.0f, -1.0f)));
        mSphere->Draw();

        UpdateMaterial(kSilver);
        UpdateMatrices(glm::translate(model, glm::vec3(3.0f, 1.0f, -1.0f)));
        mSphere->Draw();

        UpdateMaterial(mTorusMaterial);
        UpdateMatrices(mTorus->GetTransform());
        mTorus->Draw();
    }

    void SetLight(float position[3], float intensity[3], int type, size_t idx)
    {
        mLights[idx].mIntensity = glm::vec3(intensity[0], intensity[1], intensity[2]);
        mLights[idx].mPosition = glm::vec4(position[0], position[1], position[2], 1.0f);
        if (type == 0)  // Is Directional
            mLights[idx].mPosition.w = 0.0f;
    }

    void SetTorusMaterial(float color[3], float roughness, bool isMetal)
    {
        mTorusMaterial.mColor = glm::vec3(color[0], color[1], color[2]);
        mTorusMaterial.mRoughness = roughness;
        mTorusMaterial.mIsMetal = isMetal;
    }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override { glViewport(0, 0, width, height); }

private:
    struct Light
    {
        glm::vec4 mPosition;
        glm::vec3 mIntensity;
    };

    struct Material
    {
        glm::vec3 mColor;
        float mRoughness;
        bool mIsMetal;
    };

    void UpdateMaterial(float roughness, bool isMetal, glm::vec3 color)
    {
        glUniform1f(mLocations.mRoughness, roughness);
        glUniform1i(mLocations.mMetal, isMetal);
        glUniform3f(mLocations.mColor, color.r, color.g, color.b);
    }

    void UpdateMaterial(Material material)
    {
        glUniform1f(mLocations.mRoughness, material.mRoughness);
        glUniform1i(mLocations.mMetal, material.mIsMetal);
        glUniform3f(mLocations.mColor, material.mColor.r, material.mColor.g, material.mColor.b);
    }

    void UpdateLights()
    {
        glm::mat4 view = mCamera->GetTransform();

        // Light Position in View Coordinates
        for (size_t i = 0; i < mLights.size(); i++)
            mLights[i].mPosition = view * mLights[i].mPosition;

        for (size_t i = 0; i < mLights.size(); i++)
        {
            glUniform4f(mLocations.mLights[i].mPosition, mLights[i].mPosition.x, mLights[i].mPosition.y, mLights[i].mPosition.z, mLights[i].mPosition.w);
            glUniform3f(mLocations.mLights[i].mIntensity, mLights[i].mIntensity.r, mLights[i].mIntensity.g, mLights[i].mIntensity.b);
        }
    }

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

    Dazzle::RenderSystem::GL::ProgramObject mProgram;
    std::unique_ptr<Dazzle::Sphere> mSphere;
    std::unique_ptr<Dazzle::Plane> mPlane;
    std::unique_ptr<Dazzle::Torus> mTorus;
    std::shared_ptr<Camera> mCamera;
    UniformLocations mLocations;

    glm::mat4 mMVP;
    glm::mat4 mModelView;
    glm::mat3 mNormalMtx;

    // Lights
    std::array<Light, 3> mLights;

    // Materials
    Material mTorusMaterial;
    const Material kGold = { glm::vec3(1.0f, 0.71f, 0.29f), 0.43f, true };
    const Material kCopper = { glm::vec3(0.95f, 0.64f, 0.54f), 0.43f, true };
    const Material kAluminum = { glm::vec3(0.91f, 0.92f, 0.92f), 0.43f, true };
    const Material kTitanium = { glm::vec3(0.542f, 0.497f, 0.449f), 0.43f, true };
    const Material kSilver = { glm::vec3(0.95f, 0.93f, 0.88f), 0.43f, true };
};

class UIPBR : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<ScenePBR*>(scene); }
    void SetCamera(Camera* camera) override { mCamera = camera; }

    void Update() override
    {
        std::call_once(mInitFlag, &UIPBR::ResetToDefaults, this);

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
        ImGui::SeparatorText("Physically Based Rendering");
        ImGui::NewLine();

        if (ImGui::CollapsingHeader("Lights"))
        {
            ImGui::Text("L[0] Position:"); ImGui::SameLine();  ImGui::DragFloat3("##Light[0] Position", mLights[0].mPosition.data(), 0.1f, -10.0f, 10.0f);
            ImGui::Text("L[0] Intensity:"); ImGui::SameLine(); ImGui::ColorEdit3("##Light[0] Intensity", mLights[0].mIntensity.data());
            ImGui::Text("L[1] Position:"); ImGui::SameLine();  ImGui::DragFloat3("##Light[1] Position", mLights[1].mPosition.data(), 0.1f, -10.0f, 10.0f);
            ImGui::Text("L[1] Intensity:"); ImGui::SameLine(); ImGui::ColorEdit3("##Light[1] Intensity", mLights[1].mIntensity.data());
            ImGui::Text("L[2] Position:"); ImGui::SameLine();  ImGui::DragFloat3("##Light[2] Position", mLights[2].mPosition.data(), 0.1f, -10.0f, 10.0f);
            ImGui::Text("L[2] Intensity:"); ImGui::SameLine(); ImGui::ColorEdit3("##Light[2] Intensity", mLights[2].mIntensity.data());
        }
        if (ImGui::CollapsingHeader("Torus"))
        {
            ImGui::Text("Material Color:"); ImGui::SameLine(); ImGui::ColorEdit3("##Torus Color", mTorusMaterial.mColor.data());
            ImGui::Text("Material Roughness:"); ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f); ImGui::DragFloat("##Torus Roughness", &mTorusMaterial.mRoughness, 0.001f, 0.1f, 1.0f);
            ImGui::Text("Material IsMetal:"); ImGui::SameLine(); ImGui::Checkbox("##Torus IsMetal", &mTorusMaterial.mIsMetal);
        }

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
            // Lights
            for (size_t i = 0; i < mLights.size(); i++)
            {
                mScene->SetLight(mLights[i].mPosition.data(), mLights[i].mIntensity.data(), mLights[i].mType, i);
            }

            // Torus
            mScene->SetTorusMaterial(mTorusMaterial.mColor.data(), mTorusMaterial.mRoughness, mTorusMaterial.mIsMetal);
        }
    }

private:
    void ResetToDefaults()
    {
        ResetLights();
        ResetTorus();
    }

    void ResetLights()
    {
        mLights[0].mPosition = {5.0f, 3.0f, 3.0f};
        mLights[0].mIntensity = {1.0f, 1.f, 1.0f};
        mLights[0].mType = 1;

        mLights[1].mPosition = {0.0f, 0.15f, -1.0f};
        mLights[1].mIntensity = {0.3f, 0.3f, 0.3f};
        mLights[1].mType = 0;

        mLights[2].mPosition = {-7.0f, 3.0f, 3.0f};
        mLights[2].mIntensity = {0.45f, 0.45f, 0.45f};
        mLights[2].mType = 1;
    }

    void ResetTorus()
    {
        mTorusMaterial.mColor = {0.9f, 0.5f, 0.3f};
        mTorusMaterial.mRoughness = 0.43f;
        mTorusMaterial.mIsMetal = false;
    }

    ScenePBR* mScene = nullptr;
    Camera* mCamera = nullptr;
    std::once_flag mInitFlag;

    struct Light
    {
        std::array<float, 3> mPosition;
        std::array<float, 3> mIntensity;
        int mType; // 0 = Directional, 1 = Positional
    };

    struct Material
    {
        std::array<float, 3> mColor;
        float mRoughness;
        bool mIsMetal;
    };

    // Lights
    std::array<Light, 3> mLights{};

    // Torus Material
    Material mTorusMaterial{};
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = 1280;    // Window Width
    config.height = 720;    // Window Height
    config.title = "Physically Based Rendering";    // Window Title

    auto scenePBR = std::make_unique<ScenePBR>();
    auto uiPBR = std::make_unique<UIPBR>();

    App app(config, std::move(scenePBR), std::move(uiPBR));
    app.Run();

    return 0;
}
