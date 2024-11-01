#include <array>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"

#include "RenderSystem.hpp"
#include "FileManager.hpp"
#include "Sphere.hpp"

#include "App.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "UserInterface.hpp"

class SceneDiffuse : public IScene
{
public:
    struct UniformLocations
    {
        GLuint mMVP;        // (mat4) Model View Projection
        GLuint mModelView;  // (mat4) Model View
        GLuint mNormal;     // (mat3) Normal
        GLuint mKd;         // (vec3) Diffuse Reflectivity
        GLuint mLd;         // (vec3) Light Intensity
        GLuint mLp;         // (vec4) Light Position In View Coordinates
    };

    void Initialize(const std::shared_ptr<Camera>& camera) override
    {
        // Set up OpenGL debug message callback
        Dazzle::RenderSystem::GL::SetupDebugMessageCallback();

        // -----------------------------------------------------------------------------------------
        // Variables Initialization:
        mCamera = camera;
        mKd = glm::vec3();
        mLd = glm::vec3();
        mLp = glm::vec4();

        // -----------------------------------------------------------------------------------------
        // 3D Objects for this scene:
        mSphere = std::make_unique<Dazzle::Sphere>(1.0f, 8, 8); 
        mSphere->InitializeBuffers();

        // -----------------------------------------------------------------------------------------
        // Shader Program
        // Get the source code for shaders
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\DiffuseShader.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\DiffuseShader.fs.glsl");

        // Create OpenGL shader objects and build them using the respecting source code.
        Dazzle::RenderSystem::GL::ShaderObject glVSO;
        Dazzle::RenderSystem::GL::ShaderObject glFSO;
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glVSO, GL_VERTEX_SHADER, VSSC);
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glFSO, GL_FRAGMENT_SHADER, FSSC);

        // Create and build the OpenGL program object.
        mProgram.Initialize();
        Dazzle::RenderSystem::GL::ProgramBuilder::Build(mProgram, {&glVSO, &glFSO});

        // Get Uniforms
        mLocations.mMVP = glGetUniformLocation(mProgram.GetHandle(), "MVP");                // (mat4) Model View Projection
        mLocations.mModelView = glGetUniformLocation(mProgram.GetHandle(), "ModelView");    // (mat4) Model View
        mLocations.mNormal = glGetUniformLocation(mProgram.GetHandle(), "Normal");          // (mat3) Normal
        mLocations.mKd = glGetUniformLocation(mProgram.GetHandle(), "Kd");                  // (vec3) Diffuse Reflectivity
        mLocations.mLd = glGetUniformLocation(mProgram.GetHandle(), "Ld");                  // (vec3) Light Intensity
        mLocations.mLp = glGetUniformLocation(mProgram.GetHandle(), "Lp");                  // (vec4) Light Position In View Coordinates

        // Use Diffuse Shader
        glUseProgram(mProgram.GetHandle());
    }

    void Update(double time) override
    {
        static double last = time;
        double elapsed = time - last;
        last = time;

        mSphere->Rotate(glm::vec3(1.0f, 1.0f, 0.0f), static_cast<float>(elapsed * 50));
    }

    void Render() override
    {
        glClear(GL_COLOR_BUFFER_BIT);

        // Get reference to camera
        glm::mat4 view_mtx = mCamera->GetTransform();
        glm::mat4 projection_mtx = mCamera->GetProjection();
        glm::mat4 model_mtx = mSphere->GetTransform();

        // Update Uniforms
        mModelView = view_mtx * model_mtx;
        mMVP = projection_mtx * mModelView;
        glm::mat3 normal_mtx = glm::transpose(glm::inverse(mModelView));

        mLp = view_mtx * mLp;

        glUniformMatrix4fv(mLocations.mMVP, 1, GL_FALSE, glm::value_ptr(mMVP));
        glUniformMatrix4fv(mLocations.mModelView, 1, GL_FALSE, glm::value_ptr(mModelView));
        glUniformMatrix3fv(mLocations.mNormal, 1, GL_FALSE, glm::value_ptr(normal_mtx));
        glUniform3f(mLocations.mKd, mKd.r, mKd.g, mKd.b);
        glUniform3f(mLocations.mLd, mLd.r, mLd.g, mLd.b);
        glUniform4f(mLocations.mLp, mLp.x, mLp.y, mLp.z, mLp.w);

        // Draw
        mSphere->Draw();
    }

    glm::vec3 GetSpherePosition() const { return glm::vec3(mSphere->GetTransform()[3]); }

    void SetKd(float Kd[3]) { mKd = glm::vec3(Kd[0], Kd[1], Kd[2]); }
    void SetLd(float Ld[3]) { mLd = glm::vec3(Ld[0], Ld[1], Ld[2]); }
    void SetLp(float Lp[3]) { mLp = glm::vec4(Lp[0], Lp[1], Lp[2], 1.0f); }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override { glViewport(0, 0, width, height); }

private:
    Dazzle::RenderSystem::GL::ProgramObject mProgram;
    std::unique_ptr<Dazzle::Sphere> mSphere;
    std::shared_ptr<Camera> mCamera;
    UniformLocations mLocations;

    glm::mat4 mMVP;
    glm::mat4 mModelView;
    glm::vec3 mKd;
    glm::vec3 mLd;
    glm::vec4 mLp;
};

class UIDiffuse : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneDiffuse*>(scene); }
    void SetCamera(Camera* camera) override { mCamera = camera; }

    void Update() override
    {
        // Get data from the scene
        glm::vec3 cubePosition = glm::vec3();
        if (mScene)
            cubePosition = mScene->GetSpherePosition();

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
        ImGui::SeparatorText("Diffuse Shader");
        ImGui::Text("Kd: Diffuse Reflectivity");
        ImGui::Text("Ld: Light Intensity");
        ImGui::Text("Lp: Light Position");
        ImGui::NewLine();

        {
            ImGui::Text("Kd:"); ImGui::SameLine();
            ImGui::PushItemWidth(300.0f);
            ImGui::ColorEdit3("##Diffuse Reflectivity", mKd.data());
            ImGui::PopItemWidth();
        }
        {
            ImGui::Text("Ld:"); ImGui::SameLine();
            ImGui::PushItemWidth(300.0f);
            ImGui::ColorEdit3("##Light Intensity", mLd.data());
            ImGui::PopItemWidth();
        }
        {
            ImGui::Text("Lp:"); ImGui::SameLine();
            ImGui::PushItemWidth(300.0f);
            ImGui::DragFloat3("##Light Position", mLp.data(), 0.1f, -5.0f, 5.0f);
            ImGui::PopItemWidth();
        }

        if (ImGui::Button("Reset"))
            ResetToDefaults();

        // Camera
        ImGui::SeparatorText("Camera");
        ImGui::Text("Position - X: %.2f, Y: %.2f, Z: %.2f", cameraPosition.x, cameraPosition.y, cameraPosition.z);
        ImGui::Text("Yaw: %.2f, Pitch: %.2f", cameraYaw, cameraPitch);

        // Cube
        ImGui::SeparatorText("Cube");
        ImGui::Text("Position - X: %.2f, Y: %.2f, Z: %.2f", cubePosition.x, cubePosition.y, cubePosition.z);
        ImGui::End();

        // Set data to the scene
        if (mScene)
        {
            mScene->SetKd(mKd.data());
            mScene->SetLd(mLd.data());
            mScene->SetLp(mLp.data());
        }
    }

private:
    void ResetToDefaults()
    {
        mKd = mKdDefaults;
        mLd = mLdDefaults;
        mLp = mLpDefaults;
    }

    SceneDiffuse* mScene = nullptr;
    Camera* mCamera = nullptr;

    const std::array<float, 3> mKdDefaults = {0.9f, 0.5f, 0.3f};
    const std::array<float, 3> mLdDefaults = {1.0f, 1.0f, 1.0f};
    const std::array<float, 3> mLpDefaults = {5.0f, 5.0f, -5.0f};

    std::array<float, 3> mKd{mKdDefaults};   // Diffuse Reflectivity
    std::array<float, 3> mLd{mLdDefaults};   // Light Intensity
    std::array<float, 3> mLp{mLpDefaults};   // Light Position
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = 1280;        // Window Width
    config.height = 720;        // Window Height
    config.title = "Diffuse";   // Window Title

    auto sceneDiffuse = std::make_unique<SceneDiffuse>();
    auto uiDiffuse = std::make_unique<UIDiffuse>();

    App app(config, std::move(sceneDiffuse), std::move(uiDiffuse));
    app.Run();

    return 0;
}
