#include "pch.hpp"

#include <iostream>
#include <functional>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "App.hpp"
#include "Camera.hpp"
#include "Cube.hpp"
#include "FileManager.hpp"
#include "RenderSystem.hpp"
#include "Scene.hpp"
#include "UserInterface.hpp"

struct ShaderProgram
{
    GLuint mModelViewLocation;
    GLuint mProjection;
};
ShaderProgram shader;

class Scene_Diffuse : public IScene
{
public:

    void Initialize(const std::shared_ptr<Camera>& camera) override
    {
        mCamera = camera;

        // -----------------------------------------------------------------------------------------
        // Shader Program
        // Get the source code for shaders
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\DiffuseShader.vs.glsl"); // Vertex Shader Source Code
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\DiffuseShader.fs.glsl"); // Fragment Shader Source Code

        // Create OpenGL shader objects and build them using the respecting source code.
        Dazzle::RenderSystem::GL::ShaderObject glVSO;   // Vertex Shader Object
        Dazzle::RenderSystem::GL::ShaderObject glFSO;   // Fragment Shader Object
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glVSO, GL_VERTEX_SHADER, VSSC);
        Dazzle::RenderSystem::GL::ShaderBuilder::Build(glFSO, GL_FRAGMENT_SHADER, FSSC);

        // Create and build the OpenGL program object.
        mGLDiffuseShaderProgram.Initialize();
        Dazzle::RenderSystem::GL::ProgramBuilder::Build(mGLDiffuseShaderProgram, {&glVSO, &glFSO});

        // Get Uniforms
        shader.mModelViewLocation = glGetUniformLocation(mGLDiffuseShaderProgram.GetHandle(), "mModelView");
        shader.mProjection = glGetUniformLocation(mGLDiffuseShaderProgram.GetHandle(), "mProjection");

        // -----------------------------------------------------------------------------------------
        // 3D Objects for this scene:
        mCube = std::make_unique<Dazzle::Cube>();
        mCube->SetUpBuffers(0, 0);
    }

    void Render() override
    {
        glClear(GL_COLOR_BUFFER_BIT);

        // Get reference to camera
        auto view_mtx = mCamera->GetTransform();
        auto projection_mtx = mCamera->GetProjection();
        auto model_mtx = mCube->GetTransform();

        // Use Diffuse Shader
        glUseProgram(mGLDiffuseShaderProgram.GetHandle());
        // Update Uniforms
        auto model_view_mtx = view_mtx * model_mtx;
        glUniformMatrix4fv(shader.mModelViewLocation, 1, GL_FALSE, glm::value_ptr(model_view_mtx));
        glUniformMatrix4fv(shader.mProjection, 1, GL_FALSE, glm::value_ptr(projection_mtx));
        // Draw Cube
        mCube->Draw();
    }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override
    {
        glViewport(0, 0, width, height);
    }
    void Update(double time) override
    {
        static double last = time;
        double elapsed = time - last;
        last = time;
        
        mCube->Rotate(glm::vec3(1.0f, 1.0f, 0.0f), static_cast<float>(elapsed * 50));
    }

    glm::vec3 GetCubePosition() const { return glm::vec3(mCube->GetTransform()[3]); }

private:
    Dazzle::RenderSystem::GL::ProgramObject mGLDiffuseShaderProgram;
    std::unique_ptr<Dazzle::Cube> mCube;
    std::shared_ptr<Camera> mCamera;
};

class UI_Diffuse : public IUserInterface
{
public:

    void SetScene(const std::shared_ptr<IScene>& scene) override { mScene = std::static_pointer_cast<Scene_Diffuse>(scene); }
    void SetCamera(const std::shared_ptr<Camera>& camera) override { mCamera = camera; }

    void Update() override
    {
        auto cubePosition = mScene->GetCubePosition();

        ImGui::Begin("Settings");
        ImGui::Text("Press ESC to close the application.");
        ImGui::Text("Press TAB to toggle focus mode.");

        // Shader
        ImGui::SeparatorText("Diffuse Shader");

        // Camera
        ImGui::SeparatorText("Camera");
        ImGui::Text("Position - X: %.2f, Y: %.2f, Z: %.2f", mCamera->GetPosition().x, mCamera->GetPosition().y, mCamera->GetPosition().z);
        ImGui::Text("Yaw: %.2f, Pitch: %.2f", mCamera->GetYaw(), mCamera->GetPitch());

        // Cube
        ImGui::SeparatorText("Cube");
        ImGui::Text("Position - X: %.2f, Y: %.2f, Z: %.2f", cubePosition.x, cubePosition.y, cubePosition.z);
        ImGui::End();
    }

private:
    std::shared_ptr<Scene_Diffuse> mScene;
    std::shared_ptr<Camera> mCamera;
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = 1280;        // Window Width
    config.height = 720;        // Window Height
    config.title = "Diffuse";   // Window Title

    auto sceneDiffuse = std::make_shared<Scene_Diffuse>();
    auto uiDiffuse = std::make_unique<UI_Diffuse>();

    App app(config, sceneDiffuse, std::move(uiDiffuse));
    app.Run();

    return 0;
}
