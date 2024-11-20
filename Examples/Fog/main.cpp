#include <algorithm>
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

#include "App.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "UserInterface.hpp"

class SceneFog : public IScene
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
        GLuint mLp;         // (vec4) Light Position In View Coordinates

        // Material
        GLuint mKa;         // (vec3) Ambient Reflectivity
        GLuint mKd;         // (vec3) Diffuse Reflectivity
        GLuint mKs;         // (vec3) Specular Reflectivity
        GLuint mShininess;  // (float) Shininess

        GLuint mFogDensity;
        GLuint mFogMaxDistance;
        GLuint mFogMinDistance;
        GLuint mFogColor;
        GLuint mFogModel;
    };

    SceneFog() :  mMVP(), mModelView(), mNormalMtx(), mLocations(),
                    mSKa(), mSKd(), mSKs(), mSShininess(),
                    mPKa(), mPKd(), mPKs(), mPShininess(),
                    mLa(), mLd(), mLs(), mLp() {}

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
        mPlane->SetPosition(glm::vec3(0.0f, -1.0f, -5.0f));
        mPlane->InitializeBuffers();

        mSphere = std::make_unique<Dazzle::Sphere>(); 
        mSphere->InitializeBuffers();

        // -----------------------------------------------------------------------------------------
        // Shader Program
        // Get the source code for shaders
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\Fog.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\Fog.fs.glsl");

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
        mLocations.mLp = glGetUniformLocation(mProgram.GetHandle(), "light.Lp");                    // (vec4) Position In View Coordinates
        // Uniforms - Sphere Material
        mLocations.mKa = glGetUniformLocation(mProgram.GetHandle(), "material.Ka");                 // (vec3) Ambient Intensity
        mLocations.mKd = glGetUniformLocation(mProgram.GetHandle(), "material.Kd");                 // (vec3) Diffuse Intensity
        mLocations.mKs = glGetUniformLocation(mProgram.GetHandle(), "material.Ks");                 // (vec3) Specular Intensity
        mLocations.mShininess = glGetUniformLocation(mProgram.GetHandle(), "material.Shininess");   // (float) Shininess Factor
        // Uniforms - Fog
        mLocations.mFogDensity = glGetUniformLocation(mProgram.GetHandle(), "fog.Density");
        mLocations.mFogMaxDistance = glGetUniformLocation(mProgram.GetHandle(), "fog.MaxDistance");
        mLocations.mFogMinDistance = glGetUniformLocation(mProgram.GetHandle(), "fog.MinDistance");
        mLocations.mFogColor = glGetUniformLocation(mProgram.GetHandle(), "fog.Color");
        mLocations.mFogModel = glGetUniformLocation(mProgram.GetHandle(), "fog.Model");


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
        glm::mat4 model_mtx;

        // Calculate Light position in View coordinates.
        mLp = view_mtx * mLp;

        // Update Uniforms for Light.
        glUniform3f(mLocations.mLa, mLa.r, mLa.g, mLa.b);
        glUniform3f(mLocations.mLd, mLd.r, mLd.g, mLd.b);
        glUniform3f(mLocations.mLs, mLs.r, mLs.g, mLs.b);
        glUniform4f(mLocations.mLp, mLp.x, mLp.y, mLp.z, mLp.w);

        // Update Uniforms for Fog.
        glUniform1i(mLocations.mFogModel, mFogModel);
        glUniform1f(mLocations.mFogDensity, mFogDensity);
        glUniform1f(mLocations.mFogMaxDistance, mFogMaxDistance);
        glUniform1f(mLocations.mFogMinDistance, mFogMinDistance);
        glUniform3f(mLocations.mFogColor, mFogColor.r, mFogColor.g, mFogColor.b);

        // Update Uniforms for Plane.
        model_mtx = mPlane->GetTransform();
        mModelView = view_mtx * model_mtx;
        mMVP = projection_mtx * mModelView;
        mNormalMtx = glm::transpose(glm::inverse(mModelView));

        glUniformMatrix4fv(mLocations.mMVP, 1, GL_FALSE, glm::value_ptr(mMVP));
        glUniformMatrix4fv(mLocations.mModelView, 1, GL_FALSE, glm::value_ptr(mModelView));
        glUniformMatrix3fv(mLocations.mNormal, 1, GL_FALSE, glm::value_ptr(mNormalMtx));
        glUniform3f(mLocations.mKa, mPKa.r, mPKa.g, mPKa.b);
        glUniform3f(mLocations.mKd, mPKd.r, mPKd.g, mPKd.b);
        glUniform3f(mLocations.mKs, mPKs.r, mPKs.g, mPKs.b);
        glUniform1f(mLocations.mShininess, mPShininess);
        // Draw
        mPlane->Draw();

        // Update Uniforms for Sphere.
        float distance = 0.0f;
        glUniform3f(mLocations.mKa, mSKa.r, mSKa.g, mSKa.b);
        glUniform3f(mLocations.mKd, mSKd.r, mSKd.g, mSKd.b);
        glUniform3f(mLocations.mKs, mSKs.r, mSKs.g, mSKs.b);
        glUniform1f(mLocations.mShininess, mSShininess);
        for(unsigned int i = 0; i < 4; i++)
        {
            model_mtx = mSphere->GetTransform();
            model_mtx = glm::translate(model_mtx, glm::vec3(distance * 0.6f - 1.0f, 0.0, -distance));
            distance += 7.0f;

            mModelView = view_mtx * model_mtx;
            mMVP = projection_mtx * mModelView;
            mNormalMtx = glm::transpose(glm::inverse(mModelView));

            glUniformMatrix4fv(mLocations.mMVP, 1, GL_FALSE, glm::value_ptr(mMVP));
            glUniformMatrix4fv(mLocations.mModelView, 1, GL_FALSE, glm::value_ptr(mModelView));
            glUniformMatrix3fv(mLocations.mNormal, 1, GL_FALSE, glm::value_ptr(mNormalMtx));
            // Draw
            mSphere->Draw();
        }
    }

    // Sphere
    void SetSphereKa(float Ka[3]) { mSKa = glm::vec3(Ka[0], Ka[1], Ka[2]); }
    void SetSphereKd(float Kd[3]) { mSKd = glm::vec3(Kd[0], Kd[1], Kd[2]); }
    void SetSphereKs(float Ks[3]) { mSKs = glm::vec3(Ks[0], Ks[1], Ks[2]); }
    void SetSphereShininess(float Shininess) { mSShininess = Shininess; }
    // Plane
    void SetPlaneKa(float Ka[3]) { mPKa = glm::vec3(Ka[0], Ka[1], Ka[2]); }
    void SetPlaneKd(float Kd[3]) { mPKd = glm::vec3(Kd[0], Kd[1], Kd[2]); }
    void SetPlaneKs(float Ks[3]) { mPKs = glm::vec3(Ks[0], Ks[1], Ks[2]); }
    void SetPlaneShininess(float Shininess) { mPShininess = Shininess; }
    // Light
    void SetLa(float La[3]) { mLa = glm::vec3(La[0], La[1], La[2]); }
    void SetLd(float Ld[3]) { mLd = glm::vec3(Ld[0], Ld[1], Ld[2]); }
    void SetLs(float Ls[3]) { mLs = glm::vec3(Ls[0], Ls[1], Ls[2]); }
    void SetLp(float Lp[3]) { mLp = glm::vec4(Lp[0], Lp[1], Lp[2], 1.0f); }
    // Fog
    void SetFogDensity(float density) { mFogDensity = density; }
    void SetFogMaxDistance(float maxDistance) { mFogMaxDistance = maxDistance; }
    void SetFogMinDistance(float minDistance) { mFogMinDistance = minDistance; }
    void SetFogColor(float color[3]) { mFogColor = glm::vec3(color[0], color[1], color[2]); }
    void SetFogModel(int model) { mFogModel = model; }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override { glViewport(0, 0, width, height); }

private:
    Dazzle::RenderSystem::GL::ProgramObject mProgram;
    std::unique_ptr<Dazzle::Sphere> mSphere;
    std::unique_ptr<Dazzle::Plane> mPlane;
    std::shared_ptr<Camera> mCamera;
    UniformLocations mLocations;

    glm::mat4 mMVP;
    glm::mat4 mModelView;
    glm::mat3 mNormalMtx;

    // Light
    glm::vec3 mLa;      // Light Ambient Intensity
    glm::vec3 mLd;      // Light Diffuse Intensity
    glm::vec3 mLs;      // Light Specular Intensity
    glm::vec4 mLp;      // Light Position

    // Sphere Material
    glm::vec3 mSKa;      // Material Ambient Reflectivity
    glm::vec3 mSKd;      // Material Diffuse Reflectivity
    glm::vec3 mSKs;      // Material Specular Reflectivity
    float mSShininess;   // Shininess Factor
    // Plane Material
    glm::vec3 mPKa;      // Material Ambient Reflectivity
    glm::vec3 mPKd;      // Material Diffuse Reflectivity
    glm::vec3 mPKs;      // Material Specular Reflectivity
    float mPShininess;   // Shininess Factor

    // Fog
    int mFogModel;
    float mFogDensity;
    float mFogMaxDistance;
    float mFogMinDistance;
    glm::vec3 mFogColor;
};

class UIFog : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneFog*>(scene); }
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
        ImGui::SeparatorText("Fog");

        // Variables
        ImGui::NewLine();
        ImGui::Text("Variables:");
        ImGui::Text("Lp: Light Position");
        ImGui::Text("La: Light Ambient Intensity");
        ImGui::Text("Ld: Light Diffuse Intensity");
        ImGui::Text("Ls: Light Specular Intensity");
        ImGui::Text("Ka: Material Ambient Reflectivity");
        ImGui::Text("Kd: Material Diffuse Reflectivity");
        ImGui::Text("Ks: Material Specular Reflectivity");

        ImGui::NewLine();
        ImGui::Text("Model:");
        ImGui::RadioButton("Linear", &mFogModel, 0); ImGui::SameLine();
        ImGui::RadioButton("Exponential", &mFogModel, 1); ImGui::SameLine();
        ImGui::RadioButton("Quadratic", &mFogModel, 2);
        ImGui::Text("Density:"); ImGui::SameLine(); ImGui::PushItemWidth(100.0f); ImGui::DragFloat("##Fog Density", &mFogDensity, 0.01f, 0.0f, 1.0f); ImGui::PopItemWidth();
        ImGui::Text("Max Distance:"); ImGui::SameLine(); ImGui::PushItemWidth(100.0f); ImGui::DragFloat("##Fog MaxDistance", &mFogMaxDistance, 1.0f, 20.0f, 60.0f); ImGui::PopItemWidth();
        ImGui::Text("Min Distance:"); ImGui::SameLine(); ImGui::PushItemWidth(100.0f); ImGui::DragFloat("##Fog MinDistance", &mFogMinDistance, 1.0f, 1.0f, 20.0f); ImGui::PopItemWidth();
        ImGui::Text("Color:"); ImGui::SameLine(); ImGui::ColorEdit3("##Fog Color", mFogColor.data());
        mFogDensity = std::clamp(mFogDensity, 0.0f, 1.0f);
        ImGui::NewLine();

        if (ImGui::CollapsingHeader("Light"))
        {
            ImGui::Text("Lp:"); ImGui::SameLine(); ImGui::DragFloat3("##Light Position", mLp.data(), 0.1f, -10.0f, 10.0f);
            ImGui::Text("La:"); ImGui::SameLine(); ImGui::ColorEdit3("##Ambient Intensity", mLa.data());
            ImGui::Text("Ld:"); ImGui::SameLine(); ImGui::ColorEdit3("##Diffuse Intensity", mLd.data());
            ImGui::Text("Ls:"); ImGui::SameLine(); ImGui::ColorEdit3("##Specular Intensity", mLs.data());
        }

        // Sphere Material
        if (ImGui::CollapsingHeader("Sphere"))
        {
            ImGui::Text("Ka:"); ImGui::SameLine(); ImGui::ColorEdit3("##Sphere Ambient Reflectivity", mSKa.data());
            ImGui::Text("Kd:"); ImGui::SameLine(); ImGui::ColorEdit3("##Sphere Diffuse Reflectivity", mSKd.data());
            ImGui::Text("Ks:"); ImGui::SameLine(); ImGui::ColorEdit3("##Sphere Specular Reflectivity", mSKs.data());
            ImGui::Text("Shininess:"); ImGui::SameLine(); ImGui::PushItemWidth(100.0f); ImGui::DragFloat("##Sphere Shininess", &mSShininess, 1.0f, 1.0f, 200.0f); ImGui::PopItemWidth();
        }
        // Plane Material
        if (ImGui::CollapsingHeader("Plane"))
        {
            ImGui::Text("Ka:"); ImGui::SameLine(); ImGui::ColorEdit3("##Plane Ambient Reflectivity", mPKa.data());
            ImGui::Text("Kd:"); ImGui::SameLine(); ImGui::ColorEdit3("##Plane Diffuse Reflectivity", mPKd.data());
            ImGui::Text("Ks:"); ImGui::SameLine(); ImGui::ColorEdit3("##Plane Specular Reflectivity", mPKs.data());
            ImGui::Text("Shininess:"); ImGui::SameLine(); ImGui::PushItemWidth(100.0f); ImGui::DragFloat("##Plane Shininess", &mPShininess, 1.0f, 1.0f, 300.0f); ImGui::PopItemWidth();
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
            // Sphere
            mScene->SetSphereKa(mSKa.data());
            mScene->SetSphereKd(mSKd.data());
            mScene->SetSphereKs(mSKs.data());
            mScene->SetSphereShininess(mSShininess);
            // Plane
            mScene->SetPlaneKa(mPKa.data());
            mScene->SetPlaneKd(mPKd.data());
            mScene->SetPlaneKs(mPKs.data());
            mScene->SetPlaneShininess(mPShininess);
            // Light
            mScene->SetLa(mLa.data());
            mScene->SetLd(mLd.data());
            mScene->SetLs(mLs.data());
            mScene->SetLp(mLp.data());
            // Fog
            mScene->SetFogDensity(mFogDensity);
            mScene->SetFogMaxDistance(mFogMaxDistance);
            mScene->SetFogMinDistance(mFogMinDistance);
            mScene->SetFogColor(mFogColor.data());
            mScene->SetFogModel(mFogModel);
        }
    }

private:
    void ResetToDefaults()
    {
        // Sphere
        mSKa = mSKaDefault;
        mSKd = mSKdDefault;
        mSKs = mSKsDefault;
        mSShininess = mSShininessDefault;
        // Plane
        mPKa = mPKaDefault;
        mPKd = mPKdDefault;
        mPKs = mPKsDefault;
        mPShininess = mPShininessDefault;
        // Light
        mLa = mLaDefault;
        mLd = mLdDefault;
        mLs = mLsDefault;
        mLp = mLpDefault;
        // Fog
        mFogModel = mFogModelDefault;
        mFogDensity = mFogDensityDefault;
        mFogMaxDistance = mFogMaxDistanceDefault;
        mFogMinDistance = mFogMinDistanceDefault;
        mFogColor = mFogColorDefault;
    }

    SceneFog* mScene = nullptr;
    Camera* mCamera = nullptr;

    // Light Default Values
    const std::array<float, 3> mLaDefault{0.4f, 0.4f, 0.4f};
    const std::array<float, 3> mLdDefault{1.0f, 1.0f, 1.0f};
    const std::array<float, 3> mLsDefault{1.0f, 1.0f, 1.0f};
    const std::array<float, 3> mLpDefault{5.0f, 5.0f, -2.0f};

    // Sphere's Material Default Values
    const std::array<float, 3> mSKaDefault{0.1f, 0.1f, 0.1f};
    const std::array<float, 3> mSKdDefault{0.9f, 0.5f, 0.3f};
    const std::array<float, 3> mSKsDefault{0.95f, 0.95f, 0.95f};
    const float mSShininessDefault = 100.0f;

    // Planes's Material Default Values
    const std::array<float, 3> mPKaDefault{0.75f, 0.067f, 0.067f};
    const std::array<float, 3> mPKdDefault{0.56f, 0.07f, 0.07f};
    const std::array<float, 3> mPKsDefault{0.9f, 0.9f, 0.9f};
    const float mPShininessDefault = 180.0f;

    // Light
    std::array<float, 3> mLa{mLaDefault};   // Light Ambient Intensity
    std::array<float, 3> mLd{mLdDefault};   // Light Diffuse Intensity
    std::array<float, 3> mLs{mLsDefault};   // Light Specular Intensity
    std::array<float, 3> mLp{mLpDefault};   // Light Position

    // Sphere Material
    std::array<float, 3> mSKa{mSKaDefault};   // Material Ambient Reflectivity
    std::array<float, 3> mSKd{mSKdDefault};   // Material Diffuse Reflectivity
    std::array<float, 3> mSKs{mSKsDefault};   // Material Specular Reflectivity
    float mSShininess = mSShininessDefault;   // Shininess Factor

    // Plane Material
    std::array<float, 3> mPKa{mPKaDefault};   // Material Ambient Reflectivity
    std::array<float, 3> mPKd{mPKdDefault};   // Material Diffuse Reflectivity
    std::array<float, 3> mPKs{mPKsDefault};   // Material Specular Reflectivity
    float mPShininess = mPShininessDefault;   // Shininess Factor

    // Fog's Default Values
    const int mFogModelDefault = 0;
    const float mFogDensityDefault = 0.1f;
    const float mFogMaxDistanceDefault = 30.0f;
    const float mFogMinDistanceDefault = 1.0f;
    const std::array<float, 3> mFogColorDefault{0.5f, 0.5f, 0.5f};

    // Fog
    int mFogModel = mFogModelDefault;
    float mFogDensity = mFogDensityDefault;
    float mFogMaxDistance = mFogMaxDistanceDefault;
    float mFogMinDistance = mFogMinDistanceDefault;
    std::array<float, 3> mFogColor{mFogColorDefault};
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = 1280;    // Window Width
    config.height = 720;    // Window Height
    config.title = "Fog";    // Window Title

    auto sceneFog = std::make_unique<SceneFog>();
    auto uiFog = std::make_unique<UIFog>();

    App app(config, std::move(sceneFog), std::move(uiFog));
    app.Run();

    return 0;
}
