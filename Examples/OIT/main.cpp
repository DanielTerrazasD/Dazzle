#include <array>
#include <iostream>
#include <random>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

#include "RenderSystem.hpp"
#include "FileManager.hpp"
#include "Plane.hpp"
#include "Cube.hpp"

#include "App.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "UserInterface.hpp"
#include "Utils.hpp"

#define WINDOW_WIDTH (int)1280
#define WINDOW_HEIGHT (int)720
#define TEXTURE_WIDTH (int)512
#define TEXTURE_HEIGHT (int)512

class SceneOIT : public IScene
{
public:
    struct ShaderProgram
    {
        std::unordered_map<std::string, GLuint> mLocations;
        Dazzle::RenderSystem::GL::ProgramObject mProgram;
    };

    struct Material
    {
        glm::vec4 mAmbient;
        glm::vec4 mDiffuse;
    };

    SceneOIT() :  mMVP(), mModelView(), mNormalMtx(),
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
        // 3D Objects:
        // Full Screen Quad
        mQuad = std::make_unique<Dazzle::Plane>(2.0f, 2.0f, 1, 1);
        mQuad->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        mQuad->Rotate(glm::vec3(1.0f, 0.0f, 0.0f), 90.0f);
        mQuad->InitializeBuffers();

        mBigCube = std::make_unique<Dazzle::Cube>(2.0f);
        mBigCube->InitializeBuffers();

        mSmallCube = std::make_unique<Dazzle::Cube>(0.45f);
        mSmallCube->InitializeBuffers();

        // -----------------------------------------------------------------------------------------
        // Shader Program
        InitializeShaderProgram();
        InitializeUniforms();
        InitializeShaderStorage();
    }

    void Update(double time) override
    {

    }

    void Render() override
    {
        ClearBuffers();
        Pass1();
        Pass2();
    }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override
    {
        mWidth = width; mHeight = height;
        glViewport(0, 0, mWidth, mHeight);
    }

    void SetBigCubeColor(const std::array<float, 4>& color) { mBigCubeMaterial.mDiffuse = glm::vec4(color[0], color[1], color[2], color[3]); }
    void SetSmallCubeColor(const std::array<float, 4>& color) { mSmallCubeMaterial.mDiffuse = glm::vec4(color[0], color[1], color[2], color[3]); }
    void SetLightPosition(const std::array<float, 3>& position) { mLightPosition = glm::vec4(position[0], position[1], position[2], 1.0f); }

private:

    void DrawScene()
    {
        UpdateUniforms(mCamera->GetTransform(), mCamera->GetProjection());
        UpdateMaterial(mSmallCubeMaterial);
        for (int i = 0; i <= 6; i++)
        {
            for (int j = 0; j <= 6; j++)
            {
                for (int k = 0; k <= 6; k++)
                {
                    if ((i + j + k) % 2 == 0)
                    {
                        mSmallCube->SetPosition(glm::vec3(i - 3.0, j - 3.0, k - 3.0));
                        UpdateMatrices(mShader, mSmallCube->GetTransform());
                        mSmallCube->Draw();
                    }
                }
            }
        }

        UpdateMaterial(mBigCubeMaterial);
        mBigCube->SetPosition(glm::vec3(-1.75f, -1.75f, 1.75f));
        UpdateMatrices(mShader, mBigCube->GetTransform());
        mBigCube->Draw();
        mBigCube->SetPosition(glm::vec3(-1.75f, -1.75f, -1.75f));
        UpdateMatrices(mShader, mBigCube->GetTransform());
        mBigCube->Draw();
        mBigCube->SetPosition(glm::vec3(-1.75f, 1.75f, 1.75f));
        UpdateMatrices(mShader, mBigCube->GetTransform());
        mBigCube->Draw();
        mBigCube->SetPosition(glm::vec3(-1.75f, 1.75f, -1.75f));
        UpdateMatrices(mShader, mBigCube->GetTransform());
        mBigCube->Draw();
        mBigCube->SetPosition(glm::vec3(1.75f, 1.75f, 1.75f));
        UpdateMatrices(mShader, mBigCube->GetTransform());
        mBigCube->Draw();
        mBigCube->SetPosition(glm::vec3(1.75f, 1.75f, -1.75f));
        UpdateMatrices(mShader, mBigCube->GetTransform());
        mBigCube->Draw();
        mBigCube->SetPosition(glm::vec3(1.75f, -1.75f, 1.75f));
        UpdateMatrices(mShader, mBigCube->GetTransform());
        mBigCube->Draw();
        mBigCube->SetPosition(glm::vec3(1.75f, -1.75f, -1.75f));
        UpdateMatrices(mShader, mBigCube->GetTransform());
        mBigCube->Draw();
    }

    void DrawQuad()
    {
        // Update Matrices
        auto model = mQuad->GetTransform();
        auto view = glm::mat4(1.0f);
        auto projection = glm::mat4(1.0f);
        auto modelView = view * model;
        auto MVP = projection * modelView;
        auto normal = glm::transpose(glm::inverse(glm::mat3(modelView)));
        glUniformMatrix4fv(mShader.mLocations.at("ModelView"), 1, GL_FALSE, glm::value_ptr(modelView));
        glUniformMatrix4fv(mShader.mLocations.at("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
        glUniformMatrix3fv(mShader.mLocations.at("Normal"), 1, GL_FALSE, glm::value_ptr(normal));

        // Draw
        mQuad->Draw();
    }

    // Pass #1: Render the scene and calculate the diffuse shading.
    // Store all the fragments as linked lists of nodes for every pixel of the screen.
    void Pass1()
    {
        // Use subroutine pass #1
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &mSubroutinePass1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthMask(GL_FALSE); // Disable writing to the depth buffer
        DrawScene();
        glFlush();
    }
    
    // Pass #2: For every pixel, sort all the fragments and blend them together.
    // Finally, render the result as a quad.
    void Pass2()
    {
        glMemoryBarrier( GL_SHADER_STORAGE_BARRIER_BIT ); // Ensure memory visibility and ordering for SSBOs.
        // Use subroutine pass #2
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &mSubroutinePass2);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        DrawQuad();
    }

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

    void UpdateUniforms(const glm::mat4& viewTransform, const glm::mat4& viewProjection)
    {
        glm::vec4 lightPosition;
        lightPosition = viewTransform * mLightPosition;
        glUniform4fv(mShader.mLocations.at("light.mPosition"), 1, glm::value_ptr(lightPosition));
    }

    void UpdateMaterial(const Material& material)
    {
        glUniform4fv(mShader.mLocations.at("material.mAmbient"), 1, glm::value_ptr(material.mAmbient));
        glUniform4fv(mShader.mLocations.at("material.mDiffuse"), 1, glm::value_ptr(material.mDiffuse));
    }

    void ClearBuffers()
    {
        GLuint zero = 0;
        glNamedBufferSubData(mAtomicCounterBuffer, 0, sizeof(GLuint), &zero);

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mClearBuffer);
        glTextureSubImage2D(mHeadPointerTexture, 0, 0, 0, mWidth, mHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
    }

    void InitializeShaderProgram()
    {
        // Get the source code for shaders
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\OIT.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\OIT.fs.glsl");

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
        mShader.mLocations["ModelView"] = glGetUniformLocation(handle, "ModelView");
        mShader.mLocations["MVP"] = glGetUniformLocation(handle, "MVP");
        mShader.mLocations["Normal"] = glGetUniformLocation(handle, "Normal");

        mShader.mLocations["light.mIntensity"] = glGetUniformLocation(handle, "light.mIntensity");
        mShader.mLocations["light.mPosition"] = glGetUniformLocation(handle, "light.mPosition");

        mShader.mLocations["material.mAmbient"] = glGetUniformLocation(handle, "material.mAmbient");
        mShader.mLocations["material.mDiffuse"] = glGetUniformLocation(handle, "material.mDiffuse");

        mShader.mLocations["MaxNodes"] = glGetUniformLocation(handle, "MaxNodes");
        mSubroutinePass1 = glGetSubroutineIndex(handle, GL_FRAGMENT_SHADER, "Pass1");
        mSubroutinePass2 = glGetSubroutineIndex(handle, GL_FRAGMENT_SHADER, "Pass2");

        glUseProgram(mShader.mProgram.GetHandle());
        glEnable(GL_DEPTH_TEST);
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }

    void InitializeUniforms()
    {
        // Lights
        glUniform3fv(mShader.mLocations.at("light.mIntensity"), 1, glm::value_ptr(glm::vec3(0.9f)));
        glUniform4fv(mShader.mLocations.at("light.mPosition"), 1, glm::value_ptr(glm::vec4(10.0f, 10.0f, 10.0f, 1.0f)));

        // Materials
        mBigCubeMaterial.mAmbient = glm::vec4(0.0f);
        mBigCubeMaterial.mDiffuse = glm::vec4(0.9f, 0.2f, 0.2f, 0.4f);

        mSmallCubeMaterial.mAmbient = glm::vec4(0.0f);
        mSmallCubeMaterial.mDiffuse = glm::vec4(0.2f, 0.2f, 0.9f, 0.55f);
    }

    void InitializeShaderStorage()
    {
        GLuint maxNodes = mWidth * mHeight * 20;
        GLint nodeSize = 5 * sizeof(GLfloat) + sizeof(GLuint);

        // Atomic Counter Buffer
        glCreateBuffers(1, &mAtomicCounterBuffer);
        glNamedBufferData(mAtomicCounterBuffer, sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, mAtomicCounterBuffer);

        // Image texture for head pointers
        glCreateTextures(GL_TEXTURE_2D, 1, &mHeadPointerTexture);
        glTextureStorage2D(mHeadPointerTexture, 1, GL_R32UI, mWidth, mHeight);
        glBindImageTexture(0, mHeadPointerTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

        // Linked Lists Buffer
        glCreateBuffers(1, &mLinkedListsBuffer);
        glNamedBufferData(mLinkedListsBuffer, maxNodes * nodeSize, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mLinkedListsBuffer);

        glUniform1ui(mShader.mLocations["MaxNodes"], maxNodes);

        // Clear buffer for head pointers
        std::vector<GLuint> headPtrClearBuffer(mWidth * mHeight, 0xffffffff);
        glCreateBuffers(1, &mClearBuffer);
        glNamedBufferData(mClearBuffer, headPtrClearBuffer.size() * sizeof(GLuint), headPtrClearBuffer.data(), GL_STATIC_COPY);
    }

    // Shading Programs
    ShaderProgram mShader;
    // Subroutine Indices
    GLuint mSubroutinePass1 = 0;
    GLuint mSubroutinePass2 = 0;

    // 3D Objects
    std::unique_ptr<Dazzle::Plane> mQuad;
    std::unique_ptr<Dazzle::Cube> mBigCube;
    std::unique_ptr<Dazzle::Cube> mSmallCube;
    std::shared_ptr<Camera> mCamera;

    // Materials
    Material mBigCubeMaterial;
    Material mSmallCubeMaterial;

    int mWidth; // Framebuffer Width
    int mHeight; // Framebuffer Height

    GLuint mHeadPointerTexture;
    GLuint mAtomicCounterBuffer;
    GLuint mLinkedListsBuffer;
    GLuint mClearBuffer;

    glm::vec4 mLightPosition = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    glm::mat4 mMVP;
    glm::mat4 mModelView;
    glm::mat3 mNormalMtx;
};

class UIOIT : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneOIT*>(scene); }
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
        ImGui::SeparatorText("Order-Independent Transparency (OIT)");

        ImGui::NewLine();
        ImGui::Text("Big Cube Color:"); ImGui::SameLine(); ImGui::ColorEdit4("##BigCubeColor", mBigCubeColor.data());
        ImGui::Text("Small Cube Color:"); ImGui::SameLine(); ImGui::ColorEdit4("##SmallCubeColor", mSmallCubeColor.data());
        ImGui::Text("Light Position:"); ImGui::SameLine(); ImGui::DragFloat3("", mLightPosition.data());

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
            mScene->SetBigCubeColor(mBigCubeColor);
            mScene->SetSmallCubeColor(mSmallCubeColor);
            mScene->SetLightPosition(mLightPosition);
        }
    }

private:
    void ResetToDefaults()
    {
        mBigCubeColor = {0.9f, 0.2f, 0.2f, 0.4f};
        mSmallCubeColor = {0.2f, 0.2f, 0.9f, 0.55f};
        mLightPosition = {10.0f, 10.0f, 10.0f};
    }

    SceneOIT* mScene = nullptr;
    Camera* mCamera = nullptr;

    std::array<float, 4> mBigCubeColor = {0.9f, 0.2f, 0.2f, 0.4f};
    std::array<float, 4> mSmallCubeColor = {0.2f, 0.2f, 0.9f, 0.55f};
    std::array<float, 3> mLightPosition = {10.0f, 10.0f, 10.0f};
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = WINDOW_WIDTH; // Window Width
    config.height = WINDOW_HEIGHT; // Window Height
    config.title = "Order-Independent Transparency (OIT)";    // Window Title

    auto sceneOIT = std::make_unique<SceneOIT>();
    auto uiOIT = std::make_unique<UIOIT>();

    App app(config, std::move(sceneOIT), std::move(uiOIT));
    app.Run();

    return 0;
}
