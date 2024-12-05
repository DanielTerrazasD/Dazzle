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
#include "Utils.hpp"

class SceneProjectiveTexture : public IScene
{
public:
    struct UniformLocations
    {
        GLuint mMVP;        // (mat4) Model View Projection
        GLuint mModel;      // (mat4) Model
        GLuint mModelView;  // (mat4) Model View
        GLuint mNormal;     // (mat3) Normal

        GLuint mProjectorMatrix; // (mat4)

        // Light
        GLuint mLa;         // (vec3) Ambien Intensity
        GLuint mLds;        // (vec3) Diffuse & Specular Intensity
        GLuint mLp;         // (vec4) Light Position In View Coordinates

        // Material
        GLuint mKa;         // (vec3) Ambient Reflectivity
        GLuint mKd;         // (vec3) Diffuse Reflectivity
        GLuint mKs;         // (vec3) Specular Reflectivity
        GLuint mShininess;  // (float) Shininess
    };

    SceneProjectiveTexture() :  mMVP(), mModelView(), mNormalMtx(), mLocations(),
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
        mPlane = std::make_unique<Dazzle::Plane>(); 
        mPlane->InitializeBuffers();

        mSphere = std::make_unique<Dazzle::Sphere>(1.0f, 128, 128, 0.0f, glm::pi<float>(), 0.0f, glm::two_pi<float>());
        mSphere->InitializeBuffers();

        mTorus = std::make_unique<Dazzle::Torus>(1.0f, 0.5f, 128, 128, 0.0f, glm::two_pi<float>(), 0.0f, glm::two_pi<float>());
        mTorus->InitializeBuffers();

        // -----------------------------------------------------------------------------------------
        // Shader Program
        // Get the source code for shaders
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\ProjectiveTexture.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\ProjectiveTexture.fs.glsl");

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
        GLuint flowerTexture = CreateTexture("textures\\flower.png", true);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, flowerTexture);

        // Get Uniforms
        mLocations.mMVP = glGetUniformLocation(mProgram.GetHandle(), "MVP");                        // (mat4) Model View Projection
        mLocations.mModel = glGetUniformLocation(mProgram.GetHandle(), "Model");                    // (mat4) Model
        mLocations.mModelView = glGetUniformLocation(mProgram.GetHandle(), "ModelView");            // (mat4) Model View
        mLocations.mNormal = glGetUniformLocation(mProgram.GetHandle(), "Normal");                  // (mat3) Normal
        // Uniforms - Light
        mLocations.mLa = glGetUniformLocation(mProgram.GetHandle(), "light.La");                    // (vec3) Ambient Intensity
        mLocations.mLds = glGetUniformLocation(mProgram.GetHandle(), "light.Lds");                  // (vec3) Diffuse & Specular Intensity
        mLocations.mLp = glGetUniformLocation(mProgram.GetHandle(), "light.Lp");                    // (vec4) Position In View Coordinates
        // Uniforms - Material
        mLocations.mKa = glGetUniformLocation(mProgram.GetHandle(), "material.Ka");                 // (vec3) Ambient Intensity
        mLocations.mKd = glGetUniformLocation(mProgram.GetHandle(), "material.Kd");                 // (vec3) Diffuse Intensity
        mLocations.mKs = glGetUniformLocation(mProgram.GetHandle(), "material.Ks");                 // (vec3) Specular Intensity
        mLocations.mShininess = glGetUniformLocation(mProgram.GetHandle(), "material.Shininess");   // (float) Shininess Factor
        // Uniforms - Projector
        mLocations.mProjectorMatrix = glGetUniformLocation(mProgram.GetHandle(), "ProjectorMatrix");// (mat4) 


        // Use Program Shader
        glUseProgram(mProgram.GetHandle());
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }

    void Update(double time) override {}

    void Render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get reference to camera
        glm::mat4 view_mtx = mCamera->GetTransform();
        glm::mat4 projection_mtx = mCamera->GetProjection();
        glm::mat4 model_mtx = mSphere->GetTransform();

        UpdateLights();
        UpdateProjector();

        // Sphere.
        UpdateMatrices(mSphere->GetTransform());
        glUniform3f(mLocations.mKa, mSKa.r, mSKa.g, mSKa.b);
        glUniform3f(mLocations.mKd, mSKd.r, mSKd.g, mSKd.b);
        glUniform3f(mLocations.mKs, mSKs.r, mSKs.g, mSKs.b);
        glUniform1f(mLocations.mShininess, mSShininess);
        mSphere->Draw();

        // Torus.
        UpdateMatrices(mTorus->GetTransform());
        glUniform3f(mLocations.mKa, mTKa.r, mTKa.g, mTKa.b);
        glUniform3f(mLocations.mKd, mTKd.r, mTKd.g, mTKd.b);
        glUniform3f(mLocations.mKs, mTKs.r, mTKs.g, mTKs.b);
        glUniform1f(mLocations.mShininess, mTShininess);
        mTorus->Draw();

        // Plane.
        UpdateMatrices(mPlane->GetTransform());
        glUniform3f(mLocations.mKa, mPKa.r, mPKa.g, mPKa.b);
        glUniform3f(mLocations.mKd, mPKd.r, mPKd.g, mPKd.b);
        glUniform3f(mLocations.mKs, mPKs.r, mPKs.g, mPKs.b);
        glUniform1f(mLocations.mShininess, mPShininess);
        mPlane->Draw();

    }

    void SetSpherePosition(float pos[3]) { mSphere->SetPosition(glm::vec3(pos[0], pos[1], pos[2])); }
    void SetPlanePosition(float pos[3]) { mPlane->SetPosition(glm::vec3(pos[0], pos[1], pos[2])); }
    void SetTorusPosition(float pos[3]) { mTorus->SetPosition(glm::vec3(pos[0], pos[1], pos[2])); }

    void SetProjectorPosition(float pos[3])
    {
        mProjector.mPosition = glm::vec3(pos[0], pos[1], pos[2]);
        mProjector.UpdateView();
    }

    void SetProjectorLookAt(float dir[3])
    {
        mProjector.mLookAt = glm::vec3(dir[0], dir[1], dir[2]);
        mProjector.UpdateView();
    }

    void SetProjectorFOV(float fieldOfView)
    {
        mProjector.mFOV = fieldOfView;
        mProjector.UpdateProjection();
    }

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
        glUniformMatrix4fv(mLocations.mModel, 1, GL_FALSE, glm::value_ptr(model));
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

    void UpdateProjector()
    {
        glm::mat4 projectorMatrix = mProjector.mBias * mProjector.mProjection * mProjector.mView;
        glUniformMatrix4fv(mLocations.mProjectorMatrix, 1, GL_FALSE, glm::value_ptr(projectorMatrix));
    }

    GLuint CreateTexture(const std::string& path, bool flipXY)
    {
        int width, height;
        auto data = Utils::Texture::GetTextureData(path, width, height, flipXY);
        GLuint texture = 0; // OpenGL Texture Object
        if (data)
        {
            // Create texture
            glCreateTextures(GL_TEXTURE_2D, 1, &texture);

            // Set up texture storage
            glTextureStorage2D(texture, 1, GL_RGBA8, width, height);

            // Set texture parameters
            glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

            // Upload texture data
            glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data.get());
        }

        return texture;
    }

    Dazzle::RenderSystem::GL::ProgramObject mProgram;
    UniformLocations mLocations;

    std::unique_ptr<Dazzle::Sphere> mSphere;
    std::unique_ptr<Dazzle::Plane> mPlane;
    std::unique_ptr<Dazzle::Torus> mTorus;

    std::shared_ptr<Camera> mCamera;

    glm::mat4 mMVP;
    glm::mat4 mModelView;
    glm::mat3 mNormalMtx;

    // Projector
    struct Projector
    {
        Projector()
        {
            mBias = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f));
            mBias = glm::scale(mBias, glm::vec3(0.5f));
        }

        glm::vec3 mPosition{5.0f, 5.0f, 0.0f};
        glm::vec3 mLookAt{0.0f, 0.0f, 0.0f};
        const glm::vec3 mUp{0.0f, 1.0f, 0.0f};
        glm::mat4 mView = glm::lookAt(mPosition, mLookAt, mUp);
        float mFOV = 30.0f;
        const float mAspect = 1.0f; // Square
        const float mNearPlane = 0.2f;
        const float mFarPlane = 1000.0f;
        glm::mat4 mProjection = glm::perspective(glm::radians(mFOV), mAspect, mNearPlane, mFarPlane);
        glm::mat4 mBias;

        void UpdateProjection() { mProjection = glm::perspective(glm::radians(mFOV), mAspect, mNearPlane, mFarPlane); }
        void UpdateView() { mView = glm::lookAt(mPosition, mLookAt, mUp); }
    };
    Projector mProjector;

    // Light
    glm::vec3 mLa;      // Light Ambient Intensity
    glm::vec3 mLds;     // Light Diffuse & Specular Intensity
    glm::vec4 mLp;      // Light Position

    // Sphere Material
    glm::vec3 mSKa{0.1f, 0.1f, 0.1f};       // Material Ambient Reflectivity
    glm::vec3 mSKd{0.9f, 0.5f, 0.3f};       // Material Diffuse Reflectivity
    glm::vec3 mSKs{0.95f, 0.95f, 0.95f};    // Material Specular Reflectivity
    float mSShininess = 100.0f;             // Shininess Factor

    // Torus Material
    glm::vec3 mTKa{0.1f, 0.1f, 0.1f};       // Material Ambient Reflectivity
    glm::vec3 mTKd{0.9f, 0.5f, 0.3f};       // Material Diffuse Reflectivity
    glm::vec3 mTKs{0.95f, 0.95f, 0.95f};    // Material Specular Reflectivity
    float mTShininess = 100.0f;             // Shininess Factor

    // Plane Material
    glm::vec3 mPKa{0.1f, 0.1f, 0.1f};       // Material Ambient Reflectivity
    glm::vec3 mPKd{0.7f, 0.7f, 0.7f};       // Material Diffuse Reflectivity
    glm::vec3 mPKs{0.9f, 0.9f, 0.9f};       // Material Specular Reflectivity
    float mPShininess = 180.0f;             // Shininess Factor
};

class UIProjectiveTexture : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneProjectiveTexture*>(scene); }
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
        ImGui::SeparatorText("Projective Texture Mapping");
        ImGui::Text("Variables:");
        ImGui::Text("Lp: Light Position");
        ImGui::Text("La: Light Ambient Intensity");
        ImGui::Text("Lds: Light Diffuse & Specular Intensity");

        ImGui::NewLine();
        ImGui::Text("Projector:");
        ImGui::Text("Position:"); ImGui::SameLine(); ImGui::DragFloat3("##Projector Position", mProjectorPosition.data(), 0.1f, -10.0f, 10.0f);
        ImGui::Text("Look At:"); ImGui::SameLine(); ImGui::DragFloat3("##Projector LookAt", mProjectorLookAt.data(), 0.01f, -10.0f, 10.0f);
        ImGui::Text("FOV:"); ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f); ImGui::DragFloat("##Projector FOV", &mProjectorFOV, 0.01f, 30.0f, 130.0f);

        ImGui::NewLine();
        ImGui::Text("Positional Light:");
        ImGui::Text("Lp:"); ImGui::SameLine(); ImGui::DragFloat3("##Light Position", mLp.data(), 0.1f, -10.0f, 10.0f);
        ImGui::Text("La:"); ImGui::SameLine(); ImGui::ColorEdit3("##Ambient Intensity", mLa.data());
        ImGui::Text("Lds:"); ImGui::SameLine(); ImGui::ColorEdit3("##Diffuse Intensity", mLds.data());

        // Sphere
        ImGui::NewLine();
        ImGui::Text("Sphere:");
        ImGui::Text("Position:"); ImGui::SameLine(); ImGui::DragFloat3("##Sphere Position", mSpherePosition.data(), 0.1f, -10.0f, 10.0f);
        // Plane
        ImGui::Text("Plane:");
        ImGui::Text("Position:"); ImGui::SameLine(); ImGui::DragFloat3("##Plane Position", mPlanePosition.data(), 0.1f, -10.0f, 10.0f);
        // Torus
        ImGui::Text("Torus:");
        ImGui::Text("Position:"); ImGui::SameLine(); ImGui::DragFloat3("##Torus Position", mTorusPosition.data(), 0.1f, -10.0f, 10.0f);

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
            mScene->SetLa(mLa.data());
            mScene->SetLds(mLds.data());
            mScene->SetLp(mLp.data());

            mScene->SetSpherePosition(mSpherePosition.data());
            mScene->SetPlanePosition(mPlanePosition.data());
            mScene->SetTorusPosition(mTorusPosition.data());

            mScene->SetProjectorPosition(mProjectorPosition.data());
            mScene->SetProjectorLookAt(mProjectorLookAt.data());
            mScene->SetProjectorFOV(mProjectorFOV);
        }
    }

private:
    void ResetToDefaults()
    {
        mLa = mLaDefault;
        mLds = mLdsDefault;
        mLp = mLpDefault;

        mSpherePosition = mSpherePositionDefault;
        mPlanePosition = mPlanePositionDefault;
        mTorusPosition = mTorusPositionDefault;

        mProjectorPosition = mProjectorPositionDefault;
        mProjectorLookAt = mProjectorLookAtDefault;
        mProjectorFOV = mProjectorFOVDefault;
    }

    SceneProjectiveTexture* mScene = nullptr;
    Camera* mCamera = nullptr;

    // Light Default Values
    const std::array<float, 3> mLaDefault = {0.4f, 0.4f, 0.4f};
    const std::array<float, 3> mLdsDefault = {1.0f, 1.0f, 1.0f};
    const std::array<float, 3> mLpDefault = {-5.0f, 5.0f, -2.0f};

    // Light
    std::array<float, 3> mLa{mLaDefault};   // Light Ambient Intensity
    std::array<float, 3> mLds{mLdsDefault}; // Light Diffuse & Specular Intensity
    std::array<float, 3> mLp{mLpDefault};   // Light Position

    // Projector
    const std::array<float, 3> mProjectorPositionDefault{5.0f, 5.0f, -5.0f};
    std::array<float, 3> mProjectorPosition{mProjectorPositionDefault};
    const std::array<float, 3> mProjectorLookAtDefault{-2.0f, -4.0f, -5.0f};
    std::array<float, 3> mProjectorLookAt{mProjectorLookAtDefault};
    const float mProjectorFOVDefault = 30.0f;
    float mProjectorFOV = mProjectorFOVDefault;

    // Sphere
    const std::array<float, 3> mSpherePositionDefault{-1.5f, 0.0f, -5.0f};
    std::array<float, 3> mSpherePosition = mSpherePositionDefault;
    // Plane
    const std::array<float, 3> mPlanePositionDefault{0.0f, -1.0f, -5.0f};
    std::array<float, 3> mPlanePosition = mPlanePositionDefault;
    // Torus
    const std::array<float, 3> mTorusPositionDefault{1.f, 0.0f, -5.0f};
    std::array<float, 3> mTorusPosition = mTorusPositionDefault;
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = 1280;    // Window Width
    config.height = 720;    // Window Height
    config.title = "Projective Texture Mapping";    // Window Title

    auto sceneProjectiveTexture = std::make_unique<SceneProjectiveTexture>();
    auto uiProjectiveTexture = std::make_unique<UIProjectiveTexture>();

    App app(config, std::move(sceneProjectiveTexture), std::move(uiProjectiveTexture));
    app.Run();

    return 0;
}
