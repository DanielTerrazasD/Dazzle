#include <array>
#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
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

#define WINDOW_WIDTH (int)1280
#define WINDOW_HEIGHT (int)720
#define TEXTURE_WIDTH (int)512
#define TEXTURE_HEIGHT (int)512

class SceneEDF : public IScene
{
public:
    struct ShaderProgram
    {
        std::unordered_map<std::string, GLuint> mLocations;
        Dazzle::RenderSystem::GL::ProgramObject mProgram;
    };

    SceneEDF() :  mMVP(), mModelView(), mNormalMtx(),
                    mKs(), mShininess(),
                    mLa(), mLds(), mLp(),
                    mRenderBufferTexture(), mFBO(),
                    mQuadVAO(nullptr),
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
        // Full Screen Quad
        GLfloat vertices[] =
        {
            -1.0f, -1.0f, 0.0f, // Bottom Left
            1.0f, -1.0f, 0.0f,  // Bottom Right
            1.0f, 1.0f, 0.0f,   // Top Right
            -1.0f, 1.0f, 0.0f   // Top Left
        };
        GLfloat normals[] =
        {
            0.0f, 0.0f, 1.0f,   // Bottom Left
            0.0f, 0.0f, 1.0f,   // Bottom Right
            0.0f, 0.0f, 1.0f,   // Top Right
            0.0f, 0.0f, 1.0f    // Top Left
        };
        GLint indices[] =
        {
            0, 1, 2,
            2, 3, 0
        };
        mQuadVAO = std::make_unique<Dazzle::RenderSystem::GL::VAO>();
        mQuadVBO = std::make_unique<Dazzle::RenderSystem::GL::VBO>();
        mQuadNVBO = std::make_unique<Dazzle::RenderSystem::GL::VBO>();
        mQuadEBO = std::make_unique<Dazzle::RenderSystem::GL::EBO>();

        glNamedBufferStorage(mQuadVBO->GetHandle(), 12 * sizeof(GLfloat), vertices, 0);
        glNamedBufferStorage(mQuadNVBO->GetHandle(), 12 * sizeof(GLfloat), normals, 0);
        glNamedBufferStorage(mQuadEBO->GetHandle(), 6 * sizeof(GLint), indices, 0);
        glVertexArrayVertexBuffer(mQuadVAO->GetHandle(), 0, mQuadVBO->GetHandle(), 0, 3 * sizeof(GLfloat));
        glVertexArrayAttribFormat(mQuadVAO->GetHandle(), 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(mQuadVAO->GetHandle(), 0, 0);
        glEnableVertexArrayAttrib(mQuadVAO->GetHandle(), 0);
        glVertexArrayVertexBuffer(mQuadVAO->GetHandle(), 1, mQuadNVBO->GetHandle(), 0, 3 * sizeof(float));
        glVertexArrayAttribFormat(mQuadVAO->GetHandle(), 1, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(mQuadVAO->GetHandle(), 1, 1);
        glEnableVertexArrayAttrib(mQuadVAO->GetHandle(), 1);
        glVertexArrayElementBuffer(mQuadVAO->GetHandle(), mQuadEBO->GetHandle());
        glBindVertexArray(mQuadVAO->GetHandle());

        mPlane = std::make_unique<Dazzle::Plane>(); 
        mPlane->SetPosition(glm::vec3(0.0f, -3.0f, -5.0f));
        mPlane->InitializeBuffers();

        mSphere = std::make_unique<Dazzle::Sphere>(); 
        mSphere->SetPosition(glm::vec3(1.0f, -2.0f, -5.0f));
        mSphere->InitializeBuffers();
        
        mTorus = std::make_unique<Dazzle::Torus>();
        mTorus->SetPosition(glm::vec3(-1.0f, -2.0f, -5.0f));
        mTorus->InitializeBuffers();

        // -----------------------------------------------------------------------------------------
        // Shader Program
        InitializeShaderProgram();

        // -----------------------------------------------------------------------------------------
        // Textures for this scene:

        // Use Program Shader
        glUseProgram(mShader.mProgram.GetHandle());
    }

    void Update(double time) override
    {

    }

    void Render() override
    {
        Pass1(); // Render to texture
        glFlush();
        Pass2(); // Render to screen
    }

    void SetLp(float Lp[3]) { mLp = glm::vec4(Lp[0], Lp[1], Lp[2], 1.0f); }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override
    {
        mWidth = width; mHeight = height;
        glViewport(0, 0, mWidth, mHeight);
    }

private:
    void Pass1()
    {
        // Shader Pass #1
        glUniform1i(mShader.mLocations.at("Pass"), 1);

        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Uniforms - Light
        glUniform1f(mShader.mLocations.at("EdgeThreshold"), 0.05f);
        glUniform3fv(mShader.mLocations.at("light.La"), 1, glm::value_ptr(glm::vec3(0.2f)));
        glUniform3fv(mShader.mLocations.at("light.Lds"), 1, glm::value_ptr(glm::vec3(1.0f)));
        glm::mat4 view = mCamera->GetTransform();
        mLp = view * mLp;
        glUniform4fv(mShader.mLocations.at("light.Lp"), 1, glm::value_ptr(mLp));

        glUniform3fv(mShader.mLocations.at("material.Ka"), 1, glm::value_ptr(glm::vec3(0.1f)));
        glUniform3fv(mShader.mLocations.at("material.Kd"), 1, glm::value_ptr(glm::vec3(0.9f)));
        glUniform3fv(mShader.mLocations.at("material.Ks"), 1, glm::value_ptr(glm::vec3(0.95f)));
        glUniform1f(mShader.mLocations.at("material.Shininess"), 100.0f);
        UpdateMatrices(mShader, mSphere->GetTransform());
        mSphere->Draw();

        glUniform3fv(mShader.mLocations.at("material.Ka"), 1, glm::value_ptr(glm::vec3(0.1f)));
        glUniform3fv(mShader.mLocations.at("material.Kd"), 1, glm::value_ptr(glm::vec3(0.4f)));
        glUniform3fv(mShader.mLocations.at("material.Ks"), 1, glm::value_ptr(glm::vec3(0.0f)));
        glUniform1f(mShader.mLocations.at("material.Shininess"), 1.0f);
        UpdateMatrices(mShader, mPlane->GetTransform());
        mPlane->Draw();

        glUniform3fv(mShader.mLocations.at("material.Ka"), 1, glm::value_ptr(glm::vec3(0.1f)));
        glUniform3fv(mShader.mLocations.at("material.Kd"), 1, glm::value_ptr(glm::vec3(0.9f, 0.5f, 0.2f)));
        glUniform3fv(mShader.mLocations.at("material.Ks"), 1, glm::value_ptr(glm::vec3(0.95f)));
        glUniform1f(mShader.mLocations.at("material.Shininess"), 100.0f);
        UpdateMatrices(mShader, mTorus->GetTransform());
        mTorus->Draw();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Pass2()
    {
        // Shader Pass #2
        glUniform1i(mShader.mLocations.at("Pass"), 2);

        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        // Update Matrices
        auto model = glm::mat4(1.0f);
        auto view = glm::mat4(1.0f);
        auto projection = glm::mat4(1.0f);
        auto modelView = view * model;
        auto MVP = projection * modelView;
        auto normal = glm::transpose(glm::inverse(glm::mat3(modelView)));
        glUniformMatrix4fv(mShader.mLocations.at("ModelView"), 1, GL_FALSE, glm::value_ptr(modelView));
        glUniformMatrix4fv(mShader.mLocations.at("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));
        glUniformMatrix3fv(mShader.mLocations.at("Normal"), 1, GL_FALSE, glm::value_ptr(normal));

        // Draw
        glBindVertexArray(mQuadVAO->GetHandle());
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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

    void InitializeShaderProgram()
    {
        // Get the source code for shaders
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\EdgeDetectionFilter.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\EdgeDetectionFilter.fs.glsl");

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
        mShader.mLocations["material.Ka"] = glGetUniformLocation(handle, "material.Ka");
        mShader.mLocations["material.Kd"] = glGetUniformLocation(handle, "material.Kd");
        mShader.mLocations["material.Ks"] = glGetUniformLocation(handle, "material.Ks");
        mShader.mLocations["material.Shininess"] = glGetUniformLocation(handle, "material.Shininess");
        mShader.mLocations["Pass"] = glGetUniformLocation(handle, "Pass");
        mShader.mLocations["EdgeThreshold"] = glGetUniformLocation(handle, "EdgeThreshold");
    }

    GLuint CreateFramebufferObject()
    {
        // Create Framebuffer Object
        GLuint fbo;
        glCreateFramebuffers(1, &fbo);

        // Create Texture Object
        glCreateTextures(GL_TEXTURE_2D, 1, &mRenderBufferTexture);
        glTextureStorage2D(mRenderBufferTexture, 1, GL_RGBA8, mWidth, mHeight);
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
        glNamedRenderbufferStorage(depthBuffer, GL_DEPTH_COMPONENT, mWidth, mHeight);

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

    std::unique_ptr<Dazzle::RenderSystem::GL::VAO> mQuadVAO;
    std::unique_ptr<Dazzle::RenderSystem::GL::VBO> mQuadVBO;
    std::unique_ptr<Dazzle::RenderSystem::GL::VBO> mQuadNVBO;
    std::unique_ptr<Dazzle::RenderSystem::GL::EBO> mQuadEBO;
    GLuint mRenderBufferTexture;
    GLuint mFBO;
    ShaderProgram mShader;

    std::unique_ptr<Dazzle::Plane> mPlane;
    std::unique_ptr<Dazzle::Sphere> mSphere;
    std::unique_ptr<Dazzle::Torus> mTorus;
    std::shared_ptr<Camera> mCamera;

    int mFBOTextureWidth;
    int mFBOTextureHeight;
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

class UIEDF : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneEDF*>(scene); }
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
        ImGui::SeparatorText("Edge Detection Filter");

        // Variables
        ImGui::NewLine();
        ImGui::Text("Variables:");
        ImGui::Text("Lp: Light Position");

        ImGui::NewLine();
        ImGui::Text("Positional Light:");
        ImGui::Text("Lp:"); ImGui::SameLine(); ImGui::DragFloat3("##Light Position", mLp.data(), 0.1f, -100.0f, 100.0f);

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
        }
    }

private:
    void ResetToDefaults()
    {
        mLp = mLpDefault;
    }

    SceneEDF* mScene = nullptr;
    Camera* mCamera = nullptr;

    // Light Default Values
    const std::array<float, 3> mLpDefault = {2.0f, 2.0f, 2.0f};

    // Light
    std::array<float, 3> mLp{mLpDefault};   // Light Position
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = WINDOW_WIDTH; // Window Width
    config.height = WINDOW_HEIGHT; // Window Height
    config.title = "Edge Detection Filter";    // Window Title

    auto sceneEDF = std::make_unique<SceneEDF>();
    auto uiEDF = std::make_unique<UIEDF>();

    App app(config, std::move(sceneEDF), std::move(uiEDF));
    app.Run();

    return 0;
}
