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

class SceneSpotLight : public IScene
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
        GLuint mLv;         // (vec3)
        GLuint mLe;         // (float)
        GLuint mLc;         // (float)

        // Material
        GLuint mKa;         // (vec3) Ambient Reflectivity
        GLuint mKd;         // (vec3) Diffuse Reflectivity
        GLuint mKs;         // (vec3) Specular Reflectivity
        GLuint mShininess;  // (float) Shininess
    };

    SceneSpotLight() :  mMVP(), mModelView(), mNormalMtx(), mLocations(),
                    mSKa(), mSKd(), mSKs(), mSShininess(),
                    mPKa(), mPKd(), mPKs(), mPShininess(),
                    mLa(), mLd(), mLs(), mLp(), mLc(), mLe(), mLv() {}

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

        mSphere = std::make_unique<Dazzle::Sphere>(1.0f, 128, 128, 0.0f, glm::pi<float>(), 0.0f, glm::two_pi<float>()); 
        mSphere->SetPosition(glm::vec3(-1.0f, 0.0f, -5.0f));
        mSphere->InitializeBuffers();

        mTorus = std::make_unique<Dazzle::Torus>(); 
        mTorus->SetPosition(glm::vec3(2.0f, 0.0f, -5.0f));
        mTorus->InitializeBuffers();

        // -----------------------------------------------------------------------------------------
        // Shader Program
        // Get the source code for shaders
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\SpotLight.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\SpotLight.fs.glsl");

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
        mLocations.mLv = glGetUniformLocation(mProgram.GetHandle(), "light.Lv");
        mLocations.mLe = glGetUniformLocation(mProgram.GetHandle(), "light.Le");
        mLocations.mLc = glGetUniformLocation(mProgram.GetHandle(), "light.Lc");
        // Uniforms - Sphere Material
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

    void Update(double time) override {}

    void Render() override
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get reference to camera
        glm::mat4 view_mtx = mCamera->GetTransform();
        glm::mat4 projection_mtx = mCamera->GetProjection();
        glm::mat4 model_mtx = glm::mat4();

        // Calculate Light position in View coordinates.
        mLp = view_mtx * mLp;
        // Calculate Light direction in View coordiantes.
        mLv = glm::mat3(view_mtx) * mLv;

        // Update Uniforms for Light.
        glUniform3f(mLocations.mLa, mLa.r, mLa.g, mLa.b);
        glUniform3f(mLocations.mLd, mLd.r, mLd.g, mLd.b);
        glUniform3f(mLocations.mLs, mLs.r, mLs.g, mLs.b);
        glUniform4f(mLocations.mLp, mLp.x, mLp.y, mLp.z, mLp.w);
        glUniform3f(mLocations.mLv, mLv.x, mLv.y, mLv.z);
        glUniform1f(mLocations.mLe, mLe);
        glUniform1f(mLocations.mLc, mLc);

        // Update Uniforms for Sphere.
        model_mtx = mSphere->GetTransform();
        mModelView = view_mtx * model_mtx;
        mMVP = projection_mtx * mModelView;
        mNormalMtx = glm::transpose(glm::inverse(mModelView));

        glUniformMatrix4fv(mLocations.mMVP, 1, GL_FALSE, glm::value_ptr(mMVP));
        glUniformMatrix4fv(mLocations.mModelView, 1, GL_FALSE, glm::value_ptr(mModelView));
        glUniformMatrix3fv(mLocations.mNormal, 1, GL_FALSE, glm::value_ptr(mNormalMtx));
        glUniform3f(mLocations.mKa, mSKa.r, mSKa.g, mSKa.b);
        glUniform3f(mLocations.mKd, mSKd.r, mSKd.g, mSKd.b);
        glUniform3f(mLocations.mKs, mSKs.r, mSKs.g, mSKs.b);
        glUniform1f(mLocations.mShininess, mSShininess);
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
        glUniform3f(mLocations.mKa, mSKa.r, mSKa.g, mSKa.b);
        glUniform3f(mLocations.mKd, mSKd.r, mSKd.g, mSKd.b);
        glUniform3f(mLocations.mKs, mSKs.r, mSKs.g, mSKs.b);
        glUniform1f(mLocations.mShininess, mSShininess);
        // Draw
        mTorus->Draw();

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
    }

    void SetSphereKa(float Ka[3]) { mSKa = glm::vec3(Ka[0], Ka[1], Ka[2]); }
    void SetSphereKd(float Kd[3]) { mSKd = glm::vec3(Kd[0], Kd[1], Kd[2]); }
    void SetSphereKs(float Ks[3]) { mSKs = glm::vec3(Ks[0], Ks[1], Ks[2]); }
    void SetSphereShininess(float Shininess) { mSShininess = Shininess; }
    void SetPlaneKa(float Ka[3]) { mPKa = glm::vec3(Ka[0], Ka[1], Ka[2]); }
    void SetPlaneKd(float Kd[3]) { mPKd = glm::vec3(Kd[0], Kd[1], Kd[2]); }
    void SetPlaneKs(float Ks[3]) { mPKs = glm::vec3(Ks[0], Ks[1], Ks[2]); }
    void SetPlaneShininess(float Shininess) { mPShininess = Shininess; }

    void SetLa(float La[3]) { mLa = glm::vec3(La[0], La[1], La[2]); }
    void SetLd(float Ld[3]) { mLd = glm::vec3(Ld[0], Ld[1], Ld[2]); }
    void SetLs(float Ls[3]) { mLs = glm::vec3(Ls[0], Ls[1], Ls[2]); }
    void SetLp(float Lp[3]) { mLp = glm::vec4(Lp[0], Lp[1], Lp[2], 1.0f); }
    void SetLv(float Lv[3]) { mLv = glm::vec3(Lv[0], Lv[1], Lv[2]); }
    void SetLe(float Le) { mLe = Le; }
    void SetLc(float Lc) { mLc = glm::radians(Lc); }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override { glViewport(0, 0, width, height); }

private:
    Dazzle::RenderSystem::GL::ProgramObject mProgram;
    std::unique_ptr<Dazzle::Plane> mPlane;
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
    glm::vec3 mLs;      // Light Specular Intensity
    glm::vec4 mLp;      // Light Position
    glm::vec3 mLv;      // Light Direction Vector
    float mLe;          // Light Angular Attenuation Exponent
    float mLc;          // Light Cutoff Angle

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
};

class UISpotLight : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneSpotLight*>(scene); }
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
        ImGui::SeparatorText("Blinn-Phong Reflection Model");

        // Variables
        ImGui::NewLine();
        ImGui::Text("Variables:");
        ImGui::Text("Lp: Light Position");
        ImGui::Text("La: Light Ambient Intensity");
        ImGui::Text("Ld: Light Diffuse Intensity");
        ImGui::Text("Ls: Light Specular Intensity");
        ImGui::Text("Lv: Light Direction Vector");
        ImGui::Text("Le: Light Angular Attenuation Exponent");
        ImGui::Text("Lc: Light Cutoff Angle");
        ImGui::Text("Ka: Material Ambient Reflectivity");
        ImGui::Text("Kd: Material Diffuse Reflectivity");
        ImGui::Text("Ks: Material Specular Reflectivity");

        ImGui::NewLine();
        // Spot Light
        if (ImGui::CollapsingHeader("Light"))
        {
            ImGui::Text("Lp:"); ImGui::SameLine(); ImGui::DragFloat3("##Light Position", mLp.data(), 0.1f, -10.0f, 10.0f);
            ImGui::Text("La:"); ImGui::SameLine(); ImGui::ColorEdit3("##Ambient Intensity", mLa.data());
            ImGui::Text("Ld:"); ImGui::SameLine(); ImGui::ColorEdit3("##Diffuse Intensity", mLd.data());
            ImGui::Text("Ls:"); ImGui::SameLine(); ImGui::ColorEdit3("##Specular Intensity", mLs.data());
            ImGui::Text("Lv:"); ImGui::SameLine(); ImGui::DragFloat3("##Light Direction", mLv.data(), 0.01f, -1.0f, 1.0f);
            ImGui::Text("Le:"); ImGui::SameLine(); ImGui::DragFloat("##Light Exponent", &mLe, 0.1f, 0.0f, 100.0f);
            ImGui::Text("Lc:"); ImGui::SameLine(); ImGui::DragFloat("##Light Cutoff", &mLc, 0.1f, 0.0f, 90.0f);
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

            mScene->SetLa(mLa.data());
            mScene->SetLd(mLd.data());
            mScene->SetLs(mLs.data());
            mScene->SetLp(mLp.data());
            mScene->SetLv(mLv.data());
            mScene->SetLe(mLe);
            mScene->SetLc(mLc);
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

        mLa = mLaDefault;
        mLd = mLdDefault;
        mLs = mLsDefault;
        mLp = mLpDefault;
    }

    SceneSpotLight* mScene = nullptr;
    Camera* mCamera = nullptr;

    // Spot Light Default Values
    const std::array<float, 3> mLaDefault = {0.4f, 0.4f, 0.4f};
    const std::array<float, 3> mLdDefault = {1.0f, 1.0f, 1.0f};
    const std::array<float, 3> mLsDefault = {1.0f, 1.0f, 1.0f};
    const std::array<float, 3> mLpDefault = {1.0f, 10.0f, -5.0f};
    const std::array<float, 3> mLvDefault = {0.0f, -1.0f, 0.0f};
    float mLeDefault = 50.0f;
    float mLcDefault = 15.0f;

    // Sphere's Material Default Values
    const std::array<float, 3> mSKaDefault = {0.1f, 0.1f, 0.1f};
    const std::array<float, 3> mSKdDefault = {0.9f, 0.5f, 0.3f};
    const std::array<float, 3> mSKsDefault = {0.95f, 0.95f, 0.95f};
    const float mSShininessDefault = 100.0f;

    // Planes's Material Default Values
    const std::array<float, 3> mPKaDefault = {0.1f, 0.1f, 0.1f};
    const std::array<float, 3> mPKdDefault = {0.7f, 0.7f, 0.7f};
    const std::array<float, 3> mPKsDefault = {0.9f, 0.9f, 0.9f};
    const float mPShininessDefault = 180.0f;

    // Light
    std::array<float, 3> mLa{mLaDefault};   // Light Ambient Intensity
    std::array<float, 3> mLd{mLdDefault};   // Light Diffuse Intensity
    std::array<float, 3> mLs{mLsDefault};   // Light Specular Intensity
    std::array<float, 3> mLp{mLpDefault};   // Light Position
    std::array<float, 3> mLv{mLvDefault};   // Light Direction (Vector)
    float mLe = mLeDefault;                 // Light Angular Attenuation Exponent
    float mLc = mLcDefault;                 // Light Cutoff Angle

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
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = 1280;    // Window Width
    config.height = 720;    // Window Height
    config.title = "Blinn-Phong Reflection Model";    // Window Title

    auto sceneSpotLight = std::make_unique<SceneSpotLight>();
    auto uISpotLight = std::make_unique<UISpotLight>();

    App app(config, std::move(sceneSpotLight), std::move(uISpotLight));
    app.Run();

    return 0;
}
