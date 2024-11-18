#include <array>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"

#include "RenderSystem.hpp"
#include "FileManager.hpp"
#include "Sphere.hpp"
#include "Torus.hpp"

#include "App.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "UserInterface.hpp"

class SceneToonShading : public IScene
{
public:
    struct UniformLocations
    {
        GLuint mMVP;        // (mat4) Model View Projection
        GLuint mModelView;  // (mat4) Model View
        GLuint mNormal;     // (mat3) Normal

        // Light
        GLuint mLa;         // (vec3) Ambien Intensity
        GLuint mLd;         // (vec3) Diffuse Intensity
        GLuint mLp;         // (vec4) Light Position In View Coordinates

        // Material
        GLuint mKa;         // (vec3) Ambient Reflectivity
        GLuint mKd;         // (vec3) Diffuse Reflectivity

        // Toon Shading Levels
        GLuint mLevels;
    };

    SceneToonShading() :  mMVP(), mModelView(), mNormalMtx(), mLocations(),
                    mSKa(), mSKd(),
                    mTKa(), mTKd(),
                    mLa(), mLd(), mLp() {}

    void Initialize(const std::shared_ptr<Camera>& camera) override
    {
        // Set up OpenGL debug message callback
        Dazzle::RenderSystem::GL::SetupDebugMessageCallback();

        // -----------------------------------------------------------------------------------------
        // Variables Initialization:
        mCamera = camera;

        // -----------------------------------------------------------------------------------------
        // 3D Objects for this scene:
        mTorus = std::make_unique<Dazzle::Torus>(); 
        mTorus->SetPosition(glm::vec3(2.0f, 0.0f, -5.0f));
        mTorus->InitializeBuffers();

        mSphere = std::make_unique<Dazzle::Sphere>(); 
        mSphere->SetPosition(glm::vec3(-1.0f, 0.0f, -5.0f));
        mSphere->InitializeBuffers();

        // -----------------------------------------------------------------------------------------
        // Shader Program
        // Get the source code for shaders
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\ToonShading.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\ToonShading.fs.glsl");

        // Create OpenGL shader objects and build them using the respecting source code.
        Dazzle::RenderSystem::GL::ShaderObject glVSO;
        Dazzle::RenderSystem::GL::ShaderObject glFSO;
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glVSO, GL_VERTEX_SHADER, VSSC);
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glFSO, GL_FRAGMENT_SHADER, FSSC);

        // Create and build the OpenGL program object.
        mProgram.Initialize();
        Dazzle::RenderSystem::GL::ProgramBuilder::Build(mProgram, {&glVSO, &glFSO});

        // Get Uniforms
        mLocations.mMVP = glGetUniformLocation(mProgram.GetHandle(), "MVP");                        // (mat4) Model View Projection
        mLocations.mModelView = glGetUniformLocation(mProgram.GetHandle(), "ModelView");            // (mat4) Model View
        mLocations.mNormal = glGetUniformLocation(mProgram.GetHandle(), "Normal");                  // (mat3) Normal
        // Uniforms - Light
        mLocations.mLa = glGetUniformLocation(mProgram.GetHandle(), "light.La");                    // (vec3) Ambient Intensity
        mLocations.mLd = glGetUniformLocation(mProgram.GetHandle(), "light.Ld");                    // (vec3) Diffuse Intensity
        mLocations.mLp = glGetUniformLocation(mProgram.GetHandle(), "light.Lp");                    // (vec4) Position In View Coordinates
        // Uniforms - Sphere Material
        mLocations.mKa = glGetUniformLocation(mProgram.GetHandle(), "material.Ka");                 // (vec3) Ambient Intensity
        mLocations.mKd = glGetUniformLocation(mProgram.GetHandle(), "material.Kd");                 // (vec3) Diffuse Intensity
        // Uniforms - Toon Shading Levels
        mLocations.mLevels = glGetUniformLocation(mProgram.GetHandle(), "Levels");


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

        // Get reference to camera
        glm::mat4 view_mtx = mCamera->GetTransform();
        glm::mat4 projection_mtx = mCamera->GetProjection();
        glm::mat4 model_mtx = mSphere->GetTransform();

        // Calculate Light position in View coordinates.
        mLp = view_mtx * mLp;

        // Update Uniforms for Light.
        glUniform3f(mLocations.mLa, mLa.r, mLa.g, mLa.b);
        glUniform3f(mLocations.mLd, mLd.r, mLd.g, mLd.b);
        glUniform4f(mLocations.mLp, mLp.x, mLp.y, mLp.z, mLp.w);

        // Update Toon Shading Levels
        glUniform1i(mLocations.mLevels, mLevels);

        // Update Uniforms for Sphere.
        mModelView = view_mtx * model_mtx;
        mMVP = projection_mtx * mModelView;
        mNormalMtx = glm::transpose(glm::inverse(mModelView));

        glUniformMatrix4fv(mLocations.mMVP, 1, GL_FALSE, glm::value_ptr(mMVP));
        glUniformMatrix4fv(mLocations.mModelView, 1, GL_FALSE, glm::value_ptr(mModelView));
        glUniformMatrix3fv(mLocations.mNormal, 1, GL_FALSE, glm::value_ptr(mNormalMtx));
        glUniform3f(mLocations.mKa, mSKa.r, mSKa.g, mSKa.b);
        glUniform3f(mLocations.mKd, mSKd.r, mSKd.g, mSKd.b);
        // Draw
        mSphere->Draw();

        // Update Uniforms for Torus.
        model_mtx = mTorus->GetTransform();
        mModelView = view_mtx * model_mtx;
        mMVP = projection_mtx * mModelView;
        mNormalMtx = glm::transpose(glm::inverse(mModelView));

        glUniformMatrix4fv(mLocations.mMVP, 1, GL_FALSE, glm::value_ptr(mMVP));
        glUniformMatrix4fv(mLocations.mModelView, 1, GL_FALSE, glm::value_ptr(mModelView));
        glUniformMatrix3fv(mLocations.mNormal, 1, GL_FALSE, glm::value_ptr(mNormalMtx));
        glUniform3f(mLocations.mKa, mTKa.r, mTKa.g, mTKa.b);
        glUniform3f(mLocations.mKd, mTKd.r, mTKd.g, mTKd.b);
        // Draw
        mTorus->Draw();
    }

    void SetSphereKa(float Ka[3]) { mSKa = glm::vec3(Ka[0], Ka[1], Ka[2]); }
    void SetSphereKd(float Kd[3]) { mSKd = glm::vec3(Kd[0], Kd[1], Kd[2]); }
    void SetTorusKa(float Ka[3]) { mTKa = glm::vec3(Ka[0], Ka[1], Ka[2]); }
    void SetTorusKd(float Kd[3]) { mTKd = glm::vec3(Kd[0], Kd[1], Kd[2]); }

    void SetLa(float La[3]) { mLa = glm::vec3(La[0], La[1], La[2]); }
    void SetLd(float Ld[3]) { mLd = glm::vec3(Ld[0], Ld[1], Ld[2]); }
    void SetLp(float Lp[3]) { mLp = glm::vec4(Lp[0], Lp[1], Lp[2], 1.0f); }

    void SetToonLevels(int levels) { mLevels = levels; }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override { glViewport(0, 0, width, height); }

private:
    Dazzle::RenderSystem::GL::ProgramObject mProgram;
    std::unique_ptr<Dazzle::Sphere> mSphere;
    std::unique_ptr<Dazzle::Torus> mTorus;
    std::shared_ptr<Camera> mCamera;
    UniformLocations mLocations;

    glm::mat4 mMVP;
    glm::mat4 mModelView;
    glm::mat3 mNormalMtx;

    // Light
    glm::vec3 mLa;      // Light Ambient Intensity
    glm::vec3 mLd;      // Light Diffuse Intensity
    glm::vec4 mLp;      // Light Position

    // Sphere Material
    glm::vec3 mSKa;      // Material Ambient Reflectivity
    glm::vec3 mSKd;      // Material Diffuse Reflectivity
    // Torus Material
    glm::vec3 mTKa;      // Material Ambient Reflectivity
    glm::vec3 mTKd;      // Material Diffuse Reflectivity

    // Toon Shading Levels
    int mLevels;
};

class UIToonShading : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneToonShading*>(scene); }
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
        ImGui::SeparatorText("Toon Shading");

        // Variables
        ImGui::NewLine();
        ImGui::Text("Variables:");
        ImGui::Text("Lp: Light Position");
        ImGui::Text("La: Light Ambient Intensity");
        ImGui::Text("Ld: Light Diffuse Intensity");
        ImGui::Text("Ka: Material Ambient Reflectivity");
        ImGui::Text("Kd: Material Diffuse Reflectivity");

        ImGui::NewLine();
        ImGui::Text("Toon Levels:"); ImGui::SameLine(); ImGui::PushItemWidth(100.0f); ImGui::DragInt("##Levels", &mLevels, 0.1f, 2, 10); ImGui::PopItemWidth();

        ImGui::NewLine();
        ImGui::Text("Positional Light:");
        ImGui::Text("Lp:"); ImGui::SameLine(); ImGui::DragFloat3("##Light Position", mLp.data(), 0.1f, -10.0f, 10.0f);
        ImGui::Text("La:"); ImGui::SameLine(); ImGui::ColorEdit3("##Ambient Intensity", mLa.data());
        ImGui::Text("Ld:"); ImGui::SameLine(); ImGui::ColorEdit3("##Diffuse Intensity", mLd.data());

        // Sphere Material
        ImGui::NewLine();
        ImGui::Text("Sphere:");
        ImGui::Text("Ka:"); ImGui::SameLine(); ImGui::ColorEdit3("##Sphere Ambient Reflectivity", mSKa.data());
        ImGui::Text("Kd:"); ImGui::SameLine(); ImGui::ColorEdit3("##Sphere Diffuse Reflectivity", mSKd.data());
        // Torus Material
        ImGui::NewLine();
        ImGui::Text("Torus:");
        ImGui::Text("Ka:"); ImGui::SameLine(); ImGui::ColorEdit3("##Torus Ambient Reflectivity", mTKa.data());
        ImGui::Text("Kd:"); ImGui::SameLine(); ImGui::ColorEdit3("##Torus Diffuse Reflectivity", mTKd.data());

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
            // Sphere
            mScene->SetSphereKa(mSKa.data());
            mScene->SetSphereKd(mSKd.data());
            // Torus
            mScene->SetTorusKa(mTKa.data());
            mScene->SetTorusKd(mTKd.data());

            // Light
            mScene->SetLa(mLa.data());
            mScene->SetLd(mLd.data());
            mScene->SetLp(mLp.data());

            // Toon Levels
            mScene->SetToonLevels(mLevels);
        }
    }

private:
    void ResetToDefaults()
    {
        // Sphere
        mSKa = mSKaDefault;
        mSKd = mSKdDefault;
        // Torus
        mTKa = mTKaDefault;
        mTKd = mTKdDefault;

        mLa = mLaDefault;
        mLd = mLdDefault;
        mLp = mLpDefault;

        mLevels = mLevelsDefault;
    }

    SceneToonShading* mScene = nullptr;
    Camera* mCamera = nullptr;


    // Light Default Values
    const std::array<float, 3> mLaDefault = {0.4f, 0.4f, 0.4f};
    const std::array<float, 3> mLdDefault = {1.0f, 1.0f, 1.0f};
    const std::array<float, 3> mLpDefault = {5.0f, 5.0f, -2.0f};

    // Sphere's Material Default Values
    const std::array<float, 3> mSKaDefault = {0.1f, 0.1f, 0.1f};
    const std::array<float, 3> mSKdDefault = {0.9f, 0.5f, 0.3f};

    // Torus's Material Default Values
    const std::array<float, 3> mTKaDefault = {0.1f, 0.1f, 0.1f};
    const std::array<float, 3> mTKdDefault = {0.43f, 0.14f, 0.05f};

    // Toon Shading Levels
    int mLevelsDefault = 4;
    int mLevels = mLevelsDefault;

    // Light
    std::array<float, 3> mLa{mLaDefault};   // Light Ambient Intensity
    std::array<float, 3> mLd{mLdDefault};   // Light Diffuse Intensity
    std::array<float, 3> mLp{mLpDefault};   // Light Position

    // Sphere Material
    std::array<float, 3> mSKa{mSKaDefault};   // Material Ambient Reflectivity
    std::array<float, 3> mSKd{mSKdDefault};   // Material Diffuse Reflectivity

    // Torus Material
    std::array<float, 3> mTKa{mTKaDefault};   // Material Ambient Reflectivity
    std::array<float, 3> mTKd{mTKdDefault};   // Material Diffuse Reflectivity
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = 1280;    // Window Width
    config.height = 720;    // Window Height
    config.title = "Toon Shading"; // Window Title

    auto sceneToonShading = std::make_unique<SceneToonShading>();
    auto uiToonShading = std::make_unique<UIToonShading>();

    App app(config, std::move(sceneToonShading), std::move(uiToonShading));
    app.Run();

    return 0;
}
