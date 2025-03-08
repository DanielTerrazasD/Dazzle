#include <array>
#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

#include "RenderSystem.hpp"
#include "FileManager.hpp"
#include "Torus.hpp"

#include "App.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "UserInterface.hpp"
#include "Utils.hpp"

#define WINDOW_WIDTH (int)1280
#define WINDOW_HEIGHT (int)720
#define TEXTURE_WIDTH (int)512
#define TEXTURE_HEIGHT (int)512

class SceneGamma : public IScene
{
public:
    struct ShaderProgram
    {
        std::unordered_map<std::string, GLuint> mLocations;
        Dazzle::RenderSystem::GL::ProgramObject mProgram;
    };

    SceneGamma() :  mMVP(), mModelView(), mNormalMtx(),
                    mKs(), mShininess(),
                    mLa(), mLds(), mLp(),
                    mGamma(1.0f),
                    mWidth(), mHeight() {}

    void Initialize(const std::shared_ptr<Camera>& camera) override
    {
        // Set up OpenGL debug message callback
        std::vector<GLuint> filterOutIDs = {131185, 131169};
        Dazzle::RenderSystem::GL::SetupDebugMessageCallback(filterOutIDs);

        // -----------------------------------------------------------------------------------------
        // Variables Initialization:
        mCamera = camera;
        mWidth = WINDOW_WIDTH;
        mHeight = WINDOW_HEIGHT;

        // -----------------------------------------------------------------------------------------
        // 3D Objects for this scene:
        mTorus = std::make_unique<Dazzle::Torus>();
        mTorus->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));
        mTorus->InitializeBuffers();

        // -----------------------------------------------------------------------------------------
        // Shader Program
        InitializeShaderProgram();

        // -----------------------------------------------------------------------------------------
        // Textures for this scene:

        // Use Program Shader
        glEnable(GL_DEPTH_TEST);
        glUseProgram(mShader.mProgram.GetHandle());
    }

    void Update(double time) override
    {

    }

    void Render() override
    {
        // Clear buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set Gamma factor
        glUniform1f(mShader.mLocations.at("Gamma"), mGamma);

        // Uniforms - Lights
        glUniform3fv(mShader.mLocations.at("light.La"), 1, glm::value_ptr(glm::vec3(1.0f)));
        glUniform3fv(mShader.mLocations.at("light.Lds"), 1, glm::value_ptr(glm::vec3(1.0f)));
        glm::mat4 view = mCamera->GetTransform();
        mLp = view * mLp;
        glUniform4fv(mShader.mLocations.at("light.Lp"), 1, glm::value_ptr(mLp));
        // Uniforms - Materials
        glUniform3fv(mShader.mLocations.at("material.Ka"), 1, glm::value_ptr(glm::vec3(0.0f)));
        glUniform3fv(mShader.mLocations.at("material.Kd"), 1, glm::value_ptr(glm::vec3(1.0f)));
        glUniform3fv(mShader.mLocations.at("material.Ks"), 1, glm::value_ptr(glm::vec3(0.0f)));
        glUniform1f(mShader.mLocations.at("material.Shininess"), 100.0f);

        UpdateMatrices(mShader, mTorus->GetTransform());
        mTorus->Draw();
    }

    void SetLp(float Lp[3]) { mLp = glm::vec4(Lp[0], Lp[1], Lp[2], 1.0f); }
    void SetGamma(float gamma) { mGamma = gamma; }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override
    {
        mWidth = width; mHeight = height;
        glViewport(0, 0, mWidth, mHeight);
    }

private:
    void UpdateMatrices(const ShaderProgram& shader, glm::mat4 model)
    {
        glm::mat4 view = mCamera->GetTransform();
        glm::mat4 projection = mCamera->GetProjection();

        mModelView = view * model;
        mMVP = projection * mModelView;
        mNormalMtx = glm::transpose(glm::inverse(glm::mat3(mModelView)));

        auto location = shader.mLocations.find("Model");
        if (location != shader.mLocations.end())
           glUniformMatrix4fv(location->second, 1, GL_FALSE, glm::value_ptr(model));

        location = shader.mLocations.find("ModelView");
        if (location != shader.mLocations.end())
           glUniformMatrix4fv(location->second, 1, GL_FALSE, glm::value_ptr(mModelView));

        location = shader.mLocations.find("MVP");
        if (location != shader.mLocations.end())
           glUniformMatrix4fv(location->second, 1, GL_FALSE, glm::value_ptr(mMVP));

        location = shader.mLocations.find("Normal");
        if (location != shader.mLocations.end())
           glUniformMatrix3fv(location->second, 1, GL_FALSE, glm::value_ptr(mNormalMtx));
    }

    void InitializeShaderProgram()
    {
        // Get the source code for shaders
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\GammaCorrection.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\GammaCorrection.fs.glsl");

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
        // mShader.mLocations["Model"] = glGetUniformLocation(handle, "Model");
        mShader.mLocations["ModelView"] = glGetUniformLocation(handle, "ModelView");
        mShader.mLocations["MVP"] = glGetUniformLocation(handle, "MVP");
        mShader.mLocations["Normal"] = glGetUniformLocation(handle, "Normal");

        mShader.mLocations["light.La"] = glGetUniformLocation(handle, "light.La");
        mShader.mLocations["light.Lds"] = glGetUniformLocation(handle, "light.Lds");
        mShader.mLocations["light.Lp"] = glGetUniformLocation(handle, "light.Lp");
        mShader.mLocations["material.Ka"] = glGetUniformLocation(handle, "material.Ka");
        mShader.mLocations["material.Kd"] = glGetUniformLocation(handle, "material.Kd");
        mShader.mLocations["material.Ks"] = glGetUniformLocation(handle, "material.Ks");
        mShader.mLocations["material.Shininess"] = glGetUniformLocation(handle, "material.Shininess");
        mShader.mLocations["Gamma"] = glGetUniformLocation(handle, "Gamma");
    }

    ShaderProgram mShader;
    std::unique_ptr<Dazzle::Torus> mTorus;
    std::shared_ptr<Camera> mCamera;

    float mGamma;
    int mWidth;
    int mHeight;

    glm::mat4 mMVP;
    glm::mat4 mModelView;
    glm::mat3 mNormalMtx;

    // Light
    glm::vec3 mLa;      // Light Ambient Intensity
    glm::vec3 mLds;     // Light Diffuse & Specular Intensity
    glm::vec4 mLp;      // Light Position
    // Material
    glm::vec3 mKs;      // Material Specular Reflectivity
    float mShininess;   // Shininess Factor
};

class UIGamma : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneGamma*>(scene); }
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
        ImGui::SeparatorText("Gamma Correction");

        // Variables
        ImGui::NewLine();
        ImGui::Text("Variables:");
        ImGui::Text("Lp: Positional Light");
        ImGui::NewLine();

        ImGui::Text("Lp:"); ImGui::SameLine(); ImGui::DragFloat3("##Light Position", mLp.data(), 0.1f, -100.0f, 100.0f);
        ImGui::Text("Gamma:"); ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f); ImGui::DragFloat("##Gamma", &mGamma, 0.05f, 1.0f, 2.4f);

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
            mScene->SetLp(mLp.data());
            mScene->SetGamma(mGamma);
        }
    }

private:
    void ResetToDefaults()
    {
        mLp = mLpDefault;
        mGamma = kGamma;
    }

    SceneGamma* mScene = nullptr;
    Camera* mCamera = nullptr;

    // Light Default Values
    const std::array<float, 3> mLpDefault = {2.0f, 2.0f, 2.0f};
    const float kGamma = 2.2f;

    // Light
    std::array<float, 3> mLp{mLpDefault};   // Light Position
    float mGamma = kGamma;
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = WINDOW_WIDTH;        // Window Width
    config.height = WINDOW_HEIGHT;      // Window Height
    config.title = "Gamma Correction";  // Window Title

    auto sceneGamma = std::make_unique<SceneGamma>();
    auto uiGamma = std::make_unique<UIGamma>();

    App app(config, std::move(sceneGamma), std::move(uiGamma));
    app.Run();

    return 0;
}
