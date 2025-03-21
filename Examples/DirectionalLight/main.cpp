#include <array>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"

#include "RenderSystem.hpp"
#include "FileManager.hpp"
#include "Torus.hpp"

#include "App.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "UserInterface.hpp"

class SceneDirectionalLight : public IScene
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
        GLuint mLs;         // (vec3) Specular Intensity
        GLuint mLv;         // (vec3) Light Direction In View Coordinates

        // Material
        GLuint mKa;         // (vec3) Ambient Reflectivity
        GLuint mKd;         // (vec3) Diffuse Reflectivity
        GLuint mKs;         // (vec3) Specular Reflectivity
        GLuint mShininess;  // (float) Shininess
    };

    SceneDirectionalLight() :  mMVP(), mModelView(), mLocations(), mKa(), mKd(), mKs(), mShininess(),
                    mLa(), mLd(), mLs(), mLv() {}

    void Initialize(const std::shared_ptr<Camera>& camera) override
    {
        // Set up OpenGL debug message callback
        Dazzle::RenderSystem::GL::SetupDebugMessageCallback();

        // -----------------------------------------------------------------------------------------
        // Variables Initialization:
        mCamera = camera;
        mKd = glm::vec3();
        mLd = glm::vec3();
        mLv = glm::vec4();

        // -----------------------------------------------------------------------------------------
        // 3D Objects for this scene:
        mTorus = std::make_unique<Dazzle::Torus>(); 
        mTorus->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));
        mTorus->InitializeBuffers();

        // -----------------------------------------------------------------------------------------
        // Shader Program
        // Get the source code for shaders
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\DirectionalLight.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\DirectionalLight.fs.glsl");

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
        mLocations.mLs = glGetUniformLocation(mProgram.GetHandle(), "light.Ls");                    // (vec3) Specular Intensity
        mLocations.mLv = glGetUniformLocation(mProgram.GetHandle(), "light.Lv");                    // (vec3) Direction In View Coordinates
        // Uniforms - Material
        mLocations.mKa = glGetUniformLocation(mProgram.GetHandle(), "material.Ka");                 // (vec3) Ambient Intensity
        mLocations.mKd = glGetUniformLocation(mProgram.GetHandle(), "material.Kd");                 // (vec3) Diffuse Intensity
        mLocations.mKs = glGetUniformLocation(mProgram.GetHandle(), "material.Ks");                 // (vec3) Specular Intensity
        mLocations.mShininess = glGetUniformLocation(mProgram.GetHandle(), "material.Shininess");   // (float) Shininess Factor

        // Use Program Shader
        glUseProgram(mProgram.GetHandle());
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    void Update(double time) override
    {
        static double last = time;
        double elapsed = time - last;
        last = time;

        mTorus->Rotate(glm::vec3(1.0f, 1.0f, 0.0f), static_cast<float>(elapsed * 50));
    }

    void Render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get reference to camera
        glm::mat4 view_mtx = mCamera->GetTransform();
        glm::mat4 projection_mtx = mCamera->GetProjection();
        glm::mat4 model_mtx = mTorus->GetTransform();

        // Update Uniforms
        mModelView = view_mtx * model_mtx;
        mMVP = projection_mtx * mModelView;
        glm::mat3 normal_mtx = glm::transpose(glm::inverse(mModelView));

        mLv = glm::mat3(view_mtx) * mLv;

        glUniformMatrix4fv(mLocations.mMVP, 1, GL_FALSE, glm::value_ptr(mMVP));
        glUniformMatrix4fv(mLocations.mModelView, 1, GL_FALSE, glm::value_ptr(mModelView));
        glUniformMatrix3fv(mLocations.mNormal, 1, GL_FALSE, glm::value_ptr(normal_mtx));

        // Uniforms - Light
        glUniform3f(mLocations.mLa, mLa.r, mLa.g, mLa.b);
        glUniform3f(mLocations.mLd, mLd.r, mLd.g, mLd.b);
        glUniform3f(mLocations.mLs, mLs.r, mLs.g, mLs.b);
        glUniform3f(mLocations.mLv, mLv.x, mLv.y, mLv.z);
        // Uniforms - Material
        glUniform3f(mLocations.mKa, mKa.r, mKa.g, mKa.b);
        glUniform3f(mLocations.mKd, mKd.r, mKd.g, mKd.b);
        glUniform3f(mLocations.mKs, mKs.r, mKs.g, mKs.b);
        glUniform1f(mLocations.mShininess, mShininess);

        // Draw
        mTorus->Draw();
    }

    glm::vec3 GetTorusPosition() const { return glm::vec3(mTorus->GetTransform()[3]); }

    void SetKa(float Ka[3]) { mKa = glm::vec3(Ka[0], Ka[1], Ka[2]); }
    void SetKd(float Kd[3]) { mKd = glm::vec3(Kd[0], Kd[1], Kd[2]); }
    void SetKs(float Ks[3]) { mKs = glm::vec3(Ks[0], Ks[1], Ks[2]); }
    void SetShininess(float Shininess) { mShininess = Shininess; }

    void SetLa(float La[3]) { mLa = glm::vec3(La[0], La[1], La[2]); }
    void SetLd(float Ld[3]) { mLd = glm::vec3(Ld[0], Ld[1], Ld[2]); }
    void SetLs(float Ls[3]) { mLs = glm::vec3(Ls[0], Ls[1], Ls[2]); }
    void SetLp(float Lp[3]) { mLv = glm::vec3(Lp[0], Lp[1], Lp[2]); }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override { glViewport(0, 0, width, height); }

private:
    Dazzle::RenderSystem::GL::ProgramObject mProgram;
    std::unique_ptr<Dazzle::Torus> mTorus;
    std::shared_ptr<Camera> mCamera;
    UniformLocations mLocations;

    glm::mat4 mMVP;
    glm::mat4 mModelView;

    // Light
    glm::vec3 mLa;      // Light Ambient Intensity
    glm::vec3 mLd;      // Light Diffuse Intensity
    glm::vec3 mLs;      // Light Specular Intensity
    glm::vec3 mLv;      // Light Direction

    // Material
    glm::vec3 mKa;      // Material Ambient Reflectivity
    glm::vec3 mKd;      // Material Diffuse Reflectivity
    glm::vec3 mKs;      // Material Specular Reflectivity
    float mShininess;   // Shininess Factor
};

class UIDirectionalLight : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneDirectionalLight*>(scene); }
    void SetCamera(Camera* camera) override { mCamera = camera; }

    void Update() override
    {
        // Get data from the scene
        glm::vec3 torusPosition = glm::vec3();
        if (mScene)
            torusPosition = mScene->GetTorusPosition();

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
        ImGui::SeparatorText("Directional Light");

        // Shader - Light
        ImGui::Text("Light:");
        ImGui::Text("Lv: Vector/Direction");
        ImGui::Text("La: Ambient Intensity");
        ImGui::Text("Ld: Diffuse Intensity");
        ImGui::Text("Ls: Specular Intensity");
        ImGui::NewLine();
        ImGui::Text("La:"); ImGui::SameLine(); ImGui::ColorEdit3("##Ambient Intensity", mLa.data());
        ImGui::Text("Ld:"); ImGui::SameLine(); ImGui::ColorEdit3("##Diffuse Intensity", mLd.data());
        ImGui::Text("Ls:"); ImGui::SameLine(); ImGui::ColorEdit3("##Specular Intensity", mLs.data());
        ImGui::Text("Lv:"); ImGui::SameLine(); ImGui::DragFloat3("##Light Direction", mLv.data(), 0.01f, -1.0f, 1.0f);
        ImGui::NewLine();

        // Shader - Material
        ImGui::Text("Material:");
        ImGui::Text("Ka: Ambient Reflectivity");
        ImGui::Text("Kd: Diffuse Reflectivity");
        ImGui::Text("Ks: Specular Reflectivity");
        ImGui::NewLine();
        ImGui::Text("Ka:"); ImGui::SameLine(); ImGui::ColorEdit3("##Ambient Reflectivity", mKa.data());
        ImGui::Text("Kd:"); ImGui::SameLine(); ImGui::ColorEdit3("##Diffuse Reflectivity", mKd.data());
        ImGui::Text("Ks:"); ImGui::SameLine(); ImGui::ColorEdit3("##Specular Reflectivity", mKs.data());
        ImGui::Text("Shininess:"); ImGui::SameLine(); ImGui::PushItemWidth(100.0f); ImGui::DragFloat("##Shininess", &mShininess, 1.0f, 1.0f, 100.0f); ImGui::PopItemWidth();

        if (ImGui::Button("Reset"))
            ResetToDefaults();

        // Camera
        ImGui::SeparatorText("Camera");
        ImGui::Text("Position - X: %.2f, Y: %.2f, Z: %.2f", cameraPosition.x, cameraPosition.y, cameraPosition.z);
        ImGui::Text("Yaw: %.2f, Pitch: %.2f", cameraYaw, cameraPitch);

        // Torus
        ImGui::SeparatorText("Torus");
        ImGui::Text("Position - X: %.2f, Y: %.2f, Z: %.2f", torusPosition.x, torusPosition.y, torusPosition.z);
        ImGui::End();

        // Set data to the scene
        if (mScene)
        {
            mScene->SetKa(mKa.data());
            mScene->SetKd(mKd.data());
            mScene->SetKs(mKs.data());
            mScene->SetShininess(mShininess);

            mScene->SetLa(mLa.data());
            mScene->SetLd(mLd.data());
            mScene->SetLs(mLs.data());
            mScene->SetLp(mLv.data());
        }
    }

private:
    void ResetToDefaults()
    {
        mKa = mKaDefault;
        mKd = mKdDefault;
        mKs = mKsDefault;
        mShininess = mShininessDefault;

        mLa = mLaDefault;
        mLd = mLdDefault;
        mLs = mLsDefault;
        mLv = mLvDefault;
    }

    SceneDirectionalLight* mScene = nullptr;
    Camera* mCamera = nullptr;

    // Light Default Values
    const std::array<float, 3> mLaDefault = {0.4f, 0.4f, 0.4f};
    const std::array<float, 3> mLdDefault = {1.0f, 1.0f, 1.0f};
    const std::array<float, 3> mLsDefault = {1.0f, 1.0f, 1.0f};
    const std::array<float, 3> mLvDefault = {1.0f, 0.0f, 0.0f};

    // Material Default Values
    const std::array<float, 3> mKaDefault = {0.9f, 0.5f, 0.3f};
    const std::array<float, 3> mKdDefault = {0.9f, 0.5f, 0.3f};
    const std::array<float, 3> mKsDefault = {0.8f, 0.8f, 0.8f};
    const float mShininessDefault = 100.0f;

    // Light
    std::array<float, 3> mLa{mLaDefault};   // Light Ambient Intensity
    std::array<float, 3> mLd{mLdDefault};   // Light Diffuse Intensity
    std::array<float, 3> mLs{mLsDefault};   // Light Specular Intensity
    std::array<float, 3> mLv{mLvDefault};   // Light Direction

    // Material
    std::array<float, 3> mKa{mKaDefault};   // Material Ambient Reflectivity
    std::array<float, 3> mKd{mKdDefault};   // Material Diffuse Reflectivity
    std::array<float, 3> mKs{mKsDefault};   // Material Specular Reflectivity
    float mShininess = mShininessDefault;   // Shininess Factor
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = 1280;    // Window Width
    config.height = 720;    // Window Height
    config.title = "Directional Light";    // Window Title

    auto sceneDirectionalLight = std::make_unique<SceneDirectionalLight>();
    auto uiDirectionalLight = std::make_unique<UIDirectionalLight>();

    App app(config, std::move(sceneDirectionalLight), std::move(uiDirectionalLight));
    app.Run();

    return 0;
}
