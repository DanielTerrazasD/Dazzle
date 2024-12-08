#include <array>
#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

#include "RenderSystem.hpp"
#include "FileManager.hpp"
#include "Cube.hpp"
#include "Mesh.hpp"

#include "App.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "UserInterface.hpp"
#include "Utils.hpp"

#define WINDOW_WIDTH (int)1280
#define WINDOW_HEIGHT (int)720
#define TEXTURE_WIDTH (int)512
#define TEXTURE_HEIGHT (int)512

class SceneTexture : public IScene
{
public:
    struct ShaderProgram
    {
        std::unordered_map<std::string, GLuint> mLocations;
        Dazzle::RenderSystem::GL::ProgramObject mProgram;
    };

    SceneTexture() :  mMVP(), mModelView(), mNormalMtx(),
                    mKs(), mShininess(),
                    mLa(), mLds(), mLp(),
                    mOgreTexture(), mRenderBufferTexture(), mFBO(),
                    mWidth(), mHeight(), mAngle() {}

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
        mFBO = CreateFramebufferObject();

        // -----------------------------------------------------------------------------------------
        // 3D Objects for this scene:
        mCube = std::make_unique<Dazzle::Cube>(); 
        mCube->SetPosition(glm::vec3(0.0f, 0.0f, -2.0f));
        mCube->InitializeBuffers();

        mOgre = std::make_unique<Dazzle::Mesh>();
        InitializeMesh(*mOgre, "models\\bs_ears.obj");
        mOgre->InitializeBuffers();

        // -----------------------------------------------------------------------------------------
        // Shader Program
        InitializeShaderProgram();

        // -----------------------------------------------------------------------------------------
        // Textures for this scene:
        mOgreTexture = CreateTexture("textures\\diffuse.png", true);
        // Bind Texture Unit 1 to Texture Object
        glBindTextureUnit(1, mOgreTexture);

        // Use Program Shader
        glUseProgram(mShader.mProgram.GetHandle());
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }

    void Update(double time) override
    {
        static const float kRotationSpeed = glm::pi<float>() / 3.0f;
        static float previousTime = 0.0f;
        float delta = (float)time - previousTime;
        if (previousTime == 0.0f) { delta = previousTime; }
        previousTime = (float)time;

        mAngle += kRotationSpeed * delta;
        if (mAngle > glm::two_pi<float>())
            mAngle -= glm::two_pi<float>();
    }

    void Render() override
    {
        RenderToTexture();
        RenderScene();
    }

    void SetKs(float Ks[3]) { mKs = glm::vec3(Ks[0], Ks[1], Ks[2]); }
    void SetShininess(float Shininess) { mShininess = Shininess; }

    void SetLa(float La[3]) { mLa = glm::vec3(La[0], La[1], La[2]); }
    void SetLds(float Lds[3]) { mLds = glm::vec3(Lds[0], Lds[1], Lds[2]); }
    void SetLp(float Lp[3]) { mLp = glm::vec4(Lp[0], Lp[1], Lp[2], 1.0f); }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override
    {
        mWidth = width; mHeight = height;
        glViewport(0, 0, mWidth, mHeight);
    }

private:
    void RenderToTexture()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f); // Gray Background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        constexpr glm::vec3 position{0.0f, 0.0f, 2.5f};
        constexpr glm::vec3 target{0.0f, 0.0f, 0.0f};
        constexpr glm::vec3 up{0.0f, 1.0f, 0.0f};
        auto view = glm::lookAt(position, target, up);

        constexpr float fov = 50.0f;
        constexpr float aspect = 1.0f;
        constexpr float nearPlane = 0.1f;
        constexpr float farPlane = 100.0f;
        auto projection = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);

        glm::mat4 model = mOgre->GetTransform();
        model = glm::rotate(model, mAngle, glm::vec3(0.0f, 1.0f, 0.0f));
        auto mtxModelView = view * model;
        auto mtxMVP = projection * mtxModelView;
        auto mtxNormal = glm::transpose(glm::inverse(glm::mat3(mModelView)));

        // Set Uniforms
        // Uniforms - Matrices
        glUniformMatrix4fv(mShader.mLocations.at("ModelView"), 1, GL_FALSE, glm::value_ptr(mtxModelView));
        glUniformMatrix4fv(mShader.mLocations.at("MVP"), 1, GL_FALSE, glm::value_ptr(mtxMVP));
        glUniformMatrix3fv(mShader.mLocations.at("Normal"), 1, GL_FALSE, glm::value_ptr(mtxNormal));
        // Uniforms - Light
        constexpr auto lightAmbient = glm::vec3(0.15f);
        constexpr auto lightDiffuseSpecular = glm::vec3(1.0f);
        constexpr auto lightPosition = glm::vec4(2.0f, 2.0f, 2.0f, 1.0f);
        glUniform3fv(mShader.mLocations.at("light.La"), 1, glm::value_ptr(lightAmbient));
        glUniform3fv(mShader.mLocations.at("light.Lds"), 1, glm::value_ptr(lightDiffuseSpecular));
        glUniform4fv(mShader.mLocations.at("light.Lp"), 1, glm::value_ptr(lightPosition));
        // Uniforms - Material
        constexpr float materialShininess = 100.0f;
        constexpr auto materialSpecular = glm::vec3(0.0f);
        glUniform3fv(mShader.mLocations.at("material.Ks"), 1, glm::value_ptr(materialSpecular));
        glUniform1f(mShader.mLocations.at("material.Shininess"), materialShininess);
        // Uniforms - Texture
        glUniform1i(mShader.mLocations.at("renderingTexSampler"), 1);

        // Draw
        mOgre->Draw();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void RenderScene()
    {
        glViewport(0, 0, mWidth, mHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set Uniforms
        // Uniforms - Matrices
        UpdateMatrices(mShader, mCube->GetTransform());
        // Uniforms - Light
        glm::mat4 view = mCamera->GetTransform();
        mLp = view * mLp;
        glUniform3fv(mShader.mLocations.at("light.La"), 1, glm::value_ptr(mLa));
        glUniform3fv(mShader.mLocations.at("light.Lds"), 1, glm::value_ptr(mLds));
        glUniform4fv(mShader.mLocations.at("light.Lp"), 1, glm::value_ptr(mLp));
        // Uniforms - Material
        glUniform3fv(mShader.mLocations.at("material.Ks"), 1, glm::value_ptr(mKs));
        glUniform1f(mShader.mLocations.at("material.Shininess"), mShininess);
        // Uniforms - Texture
        glUniform1i(mShader.mLocations.at("renderingTexSampler"), 0);

        // Draw
        mCube->Draw();
    }

    void UpdateMatrices(const ShaderProgram& shader, glm::mat4 model)
    {
        glm::mat4 view = mCamera->GetTransform();
        glm::mat4 projection = mCamera->GetProjection();

        mModelView = view * model;
        mMVP = projection * mModelView;
        // mNormalMtx = glm::transpose(glm::inverse(mModelView));
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

            // Upload texture data
            glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data.get());
        }

        return texture;
    }

    void InitializeMesh(Dazzle::Mesh& mesh, const std::string& path)
    {
        std::string name;
        std::vector<float> vertices, normals, textureCoords;
        Utils::Geom::GetMeshDataFromObj(path, name, vertices, normals, textureCoords);
        mesh.SetName(name);
        mesh.SetVertices(std::move(vertices));
        mesh.SetNormals(std::move(normals));
        mesh.SetTextureCoordinates(std::move(textureCoords));
    }

    void InitializeShaderProgram()
    {
        // Get the source code for shaders
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\RenderToTexture.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\RenderToTexture.fs.glsl");

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
        mShader.mLocations["light.La"] = glGetUniformLocation(handle, "light.La");
        mShader.mLocations["light.Lds"] = glGetUniformLocation(handle, "light.Lds");
        mShader.mLocations["light.Lp"] = glGetUniformLocation(handle, "light.Lp");
        mShader.mLocations["material.Ks"] = glGetUniformLocation(handle, "material.Ks");
        mShader.mLocations["material.Shininess"] = glGetUniformLocation(handle, "material.Shininess");
        mShader.mLocations["renderingTexSampler"] = glGetUniformLocation(handle, "renderingTexSampler");
    }

    GLuint CreateFramebufferObject()
    {
        // Create Framebuffer Object
        GLuint fbo;
        glCreateFramebuffers(1, &fbo);

        // Create Texture Object
        glCreateTextures(GL_TEXTURE_2D, 1, &mRenderBufferTexture);
        glTextureStorage2D(mRenderBufferTexture, 1, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT);
        glTextureParameteri(mRenderBufferTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(mRenderBufferTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // Bind Texture Unit 0 to Texture Object
        glBindTextureUnit(0, mRenderBufferTexture);

        // Bind Texture to FBO
        const int kLevel = 0;
        glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, mRenderBufferTexture, kLevel);

        // Create Depth Buffer
        GLuint depthBuffer;
        glCreateRenderbuffers(1, &depthBuffer);
        glNamedRenderbufferStorage(depthBuffer, GL_DEPTH_COMPONENT, TEXTURE_WIDTH, TEXTURE_HEIGHT);

        // Bind Depth Buffer to FBO
        glNamedFramebufferRenderbuffer(fbo, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

        // Set targets for the fragment shader output variables
        GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
        glNamedFramebufferDrawBuffers(fbo, 1, drawBuffers);

        // Check the framebuffer completeness status
        GLenum result = glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER);
        if (result != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "Framebuffer Error: " << result << '\n';

        return fbo;
    }

    GLuint mOgreTexture;
    GLuint mRenderBufferTexture;
    GLuint mFBO;
    ShaderProgram mShader;

    std::unique_ptr<Dazzle::Mesh> mOgre;
    std::unique_ptr<Dazzle::Cube> mCube;
    std::shared_ptr<Camera> mCamera;

    int mWidth;
    int mHeight;
    float mAngle;

    glm::mat4 mMVP;
    glm::mat4 mModelView;
    glm::mat3 mNormalMtx;

    // Light
    glm::vec3 mLa;      // Light Ambient Intensity
    glm::vec3 mLds;     // Light Diffuse & Specular Intensity
    glm::vec4 mLp;      // Light Position
    // Cube Material
    glm::vec3 mKs;      // Material Specular Reflectivity
    float mShininess;   // Shininess Factor
};

class UITexture : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneTexture*>(scene); }
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
        ImGui::SeparatorText("Texture 2D");

        // Variables
        ImGui::NewLine();
        ImGui::Text("Variables:");
        ImGui::Text("Lp: Light Position");
        ImGui::Text("La: Light Ambient Intensity");
        ImGui::Text("Lds: Light Diffuse & Specular Intensity");
        ImGui::Text("Ks: Material Specular Reflectivity");

        ImGui::NewLine();
        ImGui::Text("Positional Light:");
        ImGui::Text("Lp:"); ImGui::SameLine(); ImGui::DragFloat3("##Light Position", mLp.data(), 0.1f, -10.0f, 10.0f);
        ImGui::Text("La:"); ImGui::SameLine(); ImGui::ColorEdit3("##Ambient Intensity", mLa.data());
        ImGui::Text("Lds:"); ImGui::SameLine(); ImGui::ColorEdit3("##Diffuse Intensity", mLds.data());
        // Cube Material
        ImGui::NewLine();
        ImGui::Text("Cube:");
        ImGui::Text("Ks:"); ImGui::SameLine(); ImGui::ColorEdit3("##Specular Reflectivity", mKs.data());
        ImGui::Text("Shininess:"); ImGui::SameLine(); ImGui::PushItemWidth(100.0f); ImGui::DragFloat("##Shininess", &mShininess, 1.0f, 1.0f, 300.0f); ImGui::PopItemWidth();

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
            // Cube
            mScene->SetKs(mKs.data());
            mScene->SetShininess(mShininess);

            // Light
            mScene->SetLa(mLa.data());
            mScene->SetLds(mLds.data());
            mScene->SetLp(mLp.data());
        }
    }

private:
    void ResetToDefaults()
    {
        // Cube
        mKs = mKsDefault;
        mShininess = mShininessDefault;

        // Light
        mLa = mLaDefault;
        mLds = mLdsDefault;
        mLp = mLpDefault;
    }

    SceneTexture* mScene = nullptr;
    Camera* mCamera = nullptr;

    // Light Default Values
    const std::array<float, 3> mLaDefault = {0.15f, 0.15f, 0.15f};
    const std::array<float, 3> mLdsDefault = {1.0f, 1.0f, 1.0f};
    const std::array<float, 3> mLpDefault = {5.0f, 5.0f, 2.0f};

    // Cube's Material Default Values
    const std::array<float, 3> mKsDefault = {0.05f, 0.05f, 0.05f};
    const float mShininessDefault = 1.0f;

    // Light
    std::array<float, 3> mLa{mLaDefault};   // Light Ambient Intensity
    std::array<float, 3> mLds{mLdsDefault}; // Light Diffuse & Specular Intensity
    std::array<float, 3> mLp{mLpDefault};   // Light Position

    // Cube Material
    std::array<float, 3> mKs{mKsDefault};   // Material Specular Reflectivity
    float mShininess = mShininessDefault;   // Shininess Factor
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = WINDOW_WIDTH; // Window Width
    config.height = WINDOW_HEIGHT; // Window Height
    config.title = "Texture 2D";    // Window Title

    auto sceneTexture = std::make_unique<SceneTexture>();
    auto uiTexture = std::make_unique<UITexture>();

    App app(config, std::move(sceneTexture), std::move(uiTexture));
    app.Run();

    return 0;
}
