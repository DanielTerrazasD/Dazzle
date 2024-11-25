#include <array>
#include <iostream>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"

#include "RenderSystem.hpp"
#include "FileManager.hpp"
#include "Cube.hpp"
#include "Plane.hpp"
#include "Sphere.hpp"
#include "Torus.hpp"

#include "App.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "UserInterface.hpp"

class SceneDebugShaders : public IScene
{
public:
    struct FlatColorUniforms
    {
        GLuint mMVP;        // (mat4) Model View Projection
        GLuint mColor;      // (vec3) Color
    };

    struct NormalsUniforms
    {
        GLuint mMVP;        // (mat4) Model View Projection
    };

    struct TextureCoordinatesUniforms
    {
        GLuint mMVP;        // (mat4) Model View Projection
    };

    SceneDebugShaders() :  mMVP() {}

    void Initialize(const std::shared_ptr<Camera>& camera) override
    {
        // Set up OpenGL debug message callback
        Dazzle::RenderSystem::GL::SetupDebugMessageCallback();

        // -----------------------------------------------------------------------------------------
        // Variables Initialization:
        mCamera = camera;

        // -----------------------------------------------------------------------------------------
        // 3D Objects used by this scene:
        auto cube = std::make_unique<Dazzle::Cube>();
        cube->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));
        cube->InitializeBuffers();

        auto sphere = std::make_unique<Dazzle::Sphere>();
        sphere->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));
        sphere->InitializeBuffers();

        auto torus = std::make_unique<Dazzle::Torus>();
        torus->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));
        torus->InitializeBuffers();

        auto plane = std::make_unique<Dazzle::Plane>();
        plane->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));
        plane->InitializeBuffers();

        mObjects["Cube"] = std::move(cube);
        mObjects["Sphere"] = std::move(sphere);
        mObjects["Torus"] = std::move(torus);
        mObjects["Plane"] = std::move(plane);

        mSelectedObject = mObjects["Cube"].get();

        // -----------------------------------------------------------------------------------------
        // Shader Programs

        // Flat Color
        auto vsscFlatColor = Dazzle::FileManager::ReadFile("shaders\\FlatColor.vs.glsl");
        auto fsscFlatColor = Dazzle::FileManager::ReadFile("shaders\\FlatColor.fs.glsl");

        Dazzle::RenderSystem::GL::ShaderObject vsoFlatColor;
        Dazzle::RenderSystem::GL::ShaderObject fsoFlatColor;
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(vsoFlatColor, GL_VERTEX_SHADER, vsscFlatColor);
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(fsoFlatColor, GL_FRAGMENT_SHADER, fsscFlatColor);

        mFlatColorProgram.Initialize();
        Dazzle::RenderSystem::GL::ProgramBuilder::Build(mFlatColorProgram, {&vsoFlatColor, &fsoFlatColor});
        mFlatColorUniformLocations.mMVP = glGetUniformLocation(mFlatColorProgram.GetHandle(), "MVP");
        mFlatColorUniformLocations.mColor = glGetUniformLocation(mFlatColorProgram.GetHandle(), "Color");

        // Normals
        auto vsscNormals = Dazzle::FileManager::ReadFile("shaders\\Normals.vs.glsl");
        auto fsscNormals = Dazzle::FileManager::ReadFile("shaders\\Normals.fs.glsl");

        Dazzle::RenderSystem::GL::ShaderObject vsoNormals;
        Dazzle::RenderSystem::GL::ShaderObject fsoNormals;
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(vsoNormals, GL_VERTEX_SHADER, vsscNormals);
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(fsoNormals, GL_FRAGMENT_SHADER, fsscNormals);

        mNormalsProgram.Initialize();
        Dazzle::RenderSystem::GL::ProgramBuilder::Build(mNormalsProgram, {&vsoNormals, &fsoNormals});
        mNormalsUniformLocations.mMVP = glGetUniformLocation(mNormalsProgram.GetHandle(), "MVP");

        // Texture Coordinates
        auto vsscTextureCoordinates = Dazzle::FileManager::ReadFile("shaders\\TextureCoordinates.vs.glsl");
        auto fsscTextureCoordinates = Dazzle::FileManager::ReadFile("shaders\\TextureCoordinates.fs.glsl");

        Dazzle::RenderSystem::GL::ShaderObject vsoTextureCoordinates;
        Dazzle::RenderSystem::GL::ShaderObject fsoTextureCoordinates;
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(vsoTextureCoordinates, GL_VERTEX_SHADER, vsscTextureCoordinates);
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(fsoTextureCoordinates, GL_FRAGMENT_SHADER, fsscTextureCoordinates);

        mTextureCoordinatesProgram.Initialize();
        Dazzle::RenderSystem::GL::ProgramBuilder::Build(mTextureCoordinatesProgram, {&vsoTextureCoordinates, &fsoTextureCoordinates});
        mTextureCoordinatesUniformLocations.mMVP = glGetUniformLocation(mTextureCoordinatesProgram.GetHandle(), "MVP");

        // Use Program Shader
        glUseProgram(mFlatColorProgram.GetHandle());
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    void Update(double time) override {}

    void Render() override
    {
        if (mSelectedObject == nullptr)
            return;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view_mtx = mCamera->GetTransform();
        glm::mat4 projection_mtx = mCamera->GetProjection();
        glm::mat4 model_mtx = mSelectedObject->GetTransform();

        // Update Uniforms
        mMVP = projection_mtx * view_mtx * model_mtx;

        if (mSelectedProgram == "FlatColor")
        {
            glUniformMatrix4fv(mFlatColorUniformLocations.mMVP, 1, GL_FALSE, glm::value_ptr(mMVP));
            glUniform4fv(mFlatColorUniformLocations.mColor, 1, mFlatColor);
        }
        else if (mSelectedProgram == "Normals")
        {
            glUniformMatrix4fv(mNormalsUniformLocations.mMVP, 1, GL_FALSE, glm::value_ptr(mMVP));
        }
        else
        {
            glUniformMatrix4fv(mTextureCoordinatesUniformLocations.mMVP, 1, GL_FALSE, glm::value_ptr(mMVP));
        }

        // Draw
        mSelectedObject->Draw();
    }

    glm::vec3 GetObjectPosition() const { return mSelectedObject == nullptr ? glm::vec3(0.0f) : glm::vec3(mSelectedObject->GetTransform()[3]); }

    void SetPolygonMode(int mode)
    {
        switch (mode)
        {
        case 0: glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); break;
        case 1: glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
        case 2:
        default: glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
        }
    }

    void SetObjectSelection(int object)
    {
        switch (object)
        {
        case 0: mSelectedObject = mObjects.at("Cube").get(); break;
        case 1: mSelectedObject = mObjects.at("Sphere").get(); break;
        case 2: mSelectedObject = mObjects.at("Torus").get(); break;
        case 3: mSelectedObject = mObjects.at("Plane").get(); break;
        }
    }

    void SetShaderProgram(int program)
    {
        switch (program)
        {
        // Flat Color
        case 0:
            mSelectedProgram = "FlatColor";
            glUseProgram(mFlatColorProgram.GetHandle());
            break;
        // Normals
        case 1:
            mSelectedProgram = "Normals";
            glUseProgram(mNormalsProgram.GetHandle());
            break;
        // Texture Coordinates
        case 2:
            mSelectedProgram = "TextureCoordinates";
            glUseProgram(mTextureCoordinatesProgram.GetHandle());
            break;
        }
    }

    void SetFlatColor(float color[4])
    {
        mFlatColor[0] = color[0];
        mFlatColor[1] = color[1];
        mFlatColor[2] = color[2];
        mFlatColor[3] = color[3];
    }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override { glViewport(0, 0, width, height); }

private:
    Dazzle::RenderSystem::GL::ProgramObject mFlatColorProgram;
    FlatColorUniforms mFlatColorUniformLocations;
    float mFlatColor[4];

    Dazzle::RenderSystem::GL::ProgramObject mNormalsProgram;
    NormalsUniforms mNormalsUniformLocations;

    Dazzle::RenderSystem::GL::ProgramObject mTextureCoordinatesProgram;
    TextureCoordinatesUniforms mTextureCoordinatesUniformLocations;

    std::unordered_map<std::string, std::unique_ptr<Dazzle::Object3D>> mObjects;
    Dazzle::Object3D* mSelectedObject;
    std::string mSelectedProgram = "FlatColor";

    std::shared_ptr<Camera> mCamera;

    glm::mat4 mMVP;
};

class UIDebugShaders : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneDebugShaders*>(scene); }
    void SetCamera(Camera* camera) override { mCamera = camera; }

    void Update() override
    {
        // Get data from the scene
        if (mScene)
        {
        }

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

        static float column = 130.0f;
        static float comboWidth = 160.0f;

        ImGui::Begin("Settings");
        ImGui::Text("Press ESC to close the application.");
        ImGui::Text("Press SHIFT to toggle cursor capture mode.");

        // Shader
        ImGui::SeparatorText("Debug Shader");
        ImGui::Text("Polygon Mode:"); ImGui::SameLine();
        ImGui::SetCursorPosX(column); ImGui::SetNextItemWidth(comboWidth);
        ImGui::Combo("##PolygonMode", &mPolygonModeIdx, "Point\0Line\0Fill\0\0");

        ImGui::Text("3D Object:"); ImGui::SameLine();
        ImGui::SetCursorPosX(column); ImGui::SetNextItemWidth(comboWidth);
        ImGui::Combo("##Object3D", &mObjectCurrentIdx, "Cube\0Sphere\0Torus\0Plane\0\0");

        ImGui::Text("Shader Program:"); ImGui::SameLine();
        ImGui::SetCursorPosX(column); ImGui::SetNextItemWidth(comboWidth);
        ImGui::Combo("##ShaderProgram", &mProgramCurrentIdx, "Flat Color\0Normals\0Texture Coordinates\0\0");
        ImGui::NewLine();

        switch (mProgramCurrentIdx)
        {
            // Flat Color
            case 0:
                ImGui::Text("Flat Color:"); ImGui::SameLine();
                ImGui::SetNextItemWidth(200.0f);
                ImGui::ColorEdit4("##Diffuse Reflectivity", mFlatColor.data());
                break;

            // Normals
            case 1:
                break;

            // Texture Coordinates
            case 2:
                break;
        }

        // Camera
        ImGui::SeparatorText("Camera");
        ImGui::Text("Position - X: %.2f, Y: %.2f, Z: %.2f", cameraPosition.x, cameraPosition.y, cameraPosition.z);
        ImGui::Text("Yaw: %.2f, Pitch: %.2f", cameraYaw, cameraPitch);
        ImGui::End();

        // Set data to the scene
        if (mScene)
        {
            mScene->SetPolygonMode(mPolygonModeIdx);
            mScene->SetObjectSelection(mObjectCurrentIdx);
            mScene->SetShaderProgram(mProgramCurrentIdx);
            if (mProgramCurrentIdx == 0)
                mScene->SetFlatColor(mFlatColor.data());
        }
    }

private:
    SceneDebugShaders* mScene = nullptr;
    Camera* mCamera = nullptr;

    int mObjectCurrentIdx = 0;
    int mProgramCurrentIdx = 0;
    int mPolygonModeIdx = 2;

    const std::array<float, 4> kFlatColorDefault = {0.58f, 0.0f, 0.07f, 1.0f};
    std::array<float, 4> mFlatColor{kFlatColorDefault};
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = 1280;        // Window Width
    config.height = 720;        // Window Height
    config.title = "Debug Shaders";   // Window Title

    auto sceneDebugShaders = std::make_unique<SceneDebugShaders>();
    auto uiDebugShaders = std::make_unique<UIDebugShaders>();

    App app(config, std::move(sceneDebugShaders), std::move(uiDebugShaders));
    app.Run();

    return 0;
}
