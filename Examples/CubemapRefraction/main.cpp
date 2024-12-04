#include <array>
#include <filesystem>
#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

#include "RenderSystem.hpp"
#include "FileManager.hpp"
#include "Torus.hpp"
#include "Skybox.hpp"

#include "App.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "UserInterface.hpp"
#include "Utils.hpp"

class SceneCubemapRefraction : public IScene
{
public:
    struct ShaderProgram
    {
        std::unordered_map<std::string, GLuint> mLocations;
        Dazzle::RenderSystem::GL::ProgramObject mProgram;
    };

    SceneCubemapRefraction() :  mMVP(), mModelView(), mNormalMtx(),
                                mCameraPosition(), mRefractionIndex(), mReflectionFactor() {}

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
        mTorus->InitializeBuffers();
        mSkybox = std::make_unique<Dazzle::Skybox>(100.0f); 
        mSkybox->InitializeBuffers();

        // -----------------------------------------------------------------------------------------
        // Textures for this scene:
        GLuint cubemap = CreateCubemap();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

        // -----------------------------------------------------------------------------------------
        // Shader Program - Skybox
        InitializeSkyboxProgram();
        // Shader Program - Refraction
        InitializeRefractionProgram();

        // Use Program Shader
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }

    void Update(double time) override {}

    void Render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update Uniforms for Skybox.
        glUseProgram(mSkyboxShader.mProgram.GetHandle());
        UpdateMatrices(mSkyboxShader, mSkybox->GetTransform());
        mSkybox->Draw();

        // Update Uniforms for Sphere.
        glUseProgram(mRefractionShader.mProgram.GetHandle());
        mCameraPosition = mCamera->GetPosition();
        glUniform3fv(mRefractionShader.mLocations.at("CameraPosition"), 1, glm::value_ptr(mCameraPosition));
        glUniform1f(mRefractionShader.mLocations.at("RefractionIndex"), mRefractionIndex);
        glUniform1f(mRefractionShader.mLocations.at("ReflectionFactor"), mReflectionFactor);
        UpdateMatrices(mRefractionShader, mTorus->GetTransform());
        mTorus->Draw();
    }

    void SetRefractionIndex(float index) { mRefractionIndex = index; }
    void SetReflectionFactor(float factor) { mReflectionFactor = factor; }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override { glViewport(0, 0, width, height); }

private:
    void InitializeSkyboxProgram()
    {
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\Skybox.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\Skybox.fs.glsl");
        Dazzle::RenderSystem::GL::ShaderObject glVSO;
        Dazzle::RenderSystem::GL::ShaderObject glFSO;
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glVSO, GL_VERTEX_SHADER, VSSC);
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glFSO, GL_FRAGMENT_SHADER, FSSC);
        mSkyboxShader.mProgram.Initialize();
        Dazzle::RenderSystem::GL::ProgramBuilder::Build(mSkyboxShader.mProgram, {&glVSO, &glFSO});

        GLuint handle = mSkyboxShader.mProgram.GetHandle();
        mSkyboxShader.mLocations["MVP"] = glGetUniformLocation(handle, "MVP");
    }

    void InitializeRefractionProgram()
    {
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\CubemapRefraction.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\CubemapRefraction.fs.glsl");
        Dazzle::RenderSystem::GL::ShaderObject glVSO;
        Dazzle::RenderSystem::GL::ShaderObject glFSO;
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glVSO, GL_VERTEX_SHADER, VSSC);
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glFSO, GL_FRAGMENT_SHADER, FSSC);
        mRefractionShader.mProgram.Initialize();
        Dazzle::RenderSystem::GL::ProgramBuilder::Build(mRefractionShader.mProgram, {&glVSO, &glFSO});

        GLuint handle = mRefractionShader.mProgram.GetHandle();
        mRefractionShader.mLocations["Model"] = glGetUniformLocation(handle, "Model");
        mRefractionShader.mLocations["MVP"] = glGetUniformLocation(handle, "MVP");
        mRefractionShader.mLocations["CameraPosition"] = glGetUniformLocation(handle, "CameraPosition");
        mRefractionShader.mLocations["ReflectionFactor"] = glGetUniformLocation(handle, "ReflectionFactor");
        mRefractionShader.mLocations["RefractionIndex"] = glGetUniformLocation(handle, "RefractionIndex");
    }

    void UpdateMatrices(const ShaderProgram& shader, glm::mat4 model)
    {
        glm::mat4 view = mCamera->GetTransform();
        glm::mat4 projection = mCamera->GetProjection();

        mModelView = view * model;
        mMVP = projection * mModelView;
        mNormalMtx = glm::transpose(glm::inverse(mModelView));

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
           glUniformMatrix4fv(location->second, 1, GL_FALSE, glm::value_ptr(mNormalMtx));
    }

    GLuint CreateCubemap()
    {
        GLuint texture = 0; // OpenGL Texture Object
        const std::vector<std::string> files = {
            "textures\\pisa_posx.hdr",
            "textures\\pisa_negx.hdr",
            "textures\\pisa_posy.hdr",
            "textures\\pisa_negy.hdr",
            "textures\\pisa_posz.hdr",
            "textures\\pisa_negz.hdr"
        };

        for (const auto& file : files)
        {
            if (!std::filesystem::exists(file))
            {
                std::cerr << "File not found: " << file << '\n';
                return texture;
            }
        }

        // Create texture
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texture);

        bool once_flag = true;
        for (size_t i = 0; i < files.size(); ++i)
        {
            int width, height;
            auto data = Utils::Texture::GetHDRTextureData(files[i], width, height, false);

            if (once_flag)
            {
                glTextureStorage2D(texture, 1, GL_RGB32F, width, height);
                once_flag = false;
            }

            glTextureSubImage3D(texture, 0, 0, 0, (GLint)i, width, height, 1, GL_RGB, GL_FLOAT, data.get());
        }

        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        return texture;
    }

    ShaderProgram mSkyboxShader;
    ShaderProgram mRefractionShader;

    std::shared_ptr<Camera> mCamera;
    std::unique_ptr<Dazzle::Torus> mTorus;
    std::unique_ptr<Dazzle::Skybox> mSkybox;

    glm::mat4 mMVP;
    glm::mat4 mModelView;
    glm::mat3 mNormalMtx;

    glm::vec3 mCameraPosition;
    float mRefractionIndex;
    float mReflectionFactor;
};

class UICubemapRefraction : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneCubemapRefraction*>(scene); }
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
        ImGui::SeparatorText("Cubemap Refraction");

        ImGui::NewLine();
        ImGui::Text("Sphere:");
        ImGui::Text("Refraction Index:"); ImGui::SameLine(); ImGui::PushItemWidth(100.0f); ImGui::DragFloat("##Index", &mRefractionIndex, 0.01f, 0.0f, 1.0f); ImGui::PopItemWidth();
        ImGui::Text("Reflection Factor:"); ImGui::SameLine(); ImGui::PushItemWidth(100.0f); ImGui::DragFloat("##Factor", &mReflectionFactor, 0.01f, 0.0f, 1.0f); ImGui::PopItemWidth();

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
            mScene->SetRefractionIndex(mRefractionIndex);
            mScene->SetReflectionFactor(mReflectionFactor);
        }
    }

private:
    void ResetToDefaults()
    {
        mReflectionFactor = mReflectionFactorDefault;
        mRefractionIndex = mRefractionIndexDefault;
    }

    SceneCubemapRefraction* mScene = nullptr;
    Camera* mCamera = nullptr;

    float mRefractionIndexDefault = 0.94f;
    float mRefractionIndex = mRefractionIndexDefault;

    const float mReflectionFactorDefault = 0.1f;
    float mReflectionFactor = mReflectionFactorDefault;
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = 1280;    // Window Width
    config.height = 720;    // Window Height
    config.title = "Cubemap Refraction";    // Window Title

    auto sceneCubemapRefraction = std::make_unique<SceneCubemapRefraction>();
    auto uiCubemapRefraction = std::make_unique<UICubemapRefraction>();

    App app(config, std::move(sceneCubemapRefraction), std::move(uiCubemapRefraction));
    app.Run();

    return 0;
}
