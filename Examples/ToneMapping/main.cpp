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

class SceneToneMapping : public IScene
{
public:
    struct ShaderProgram
    {
        std::unordered_map<std::string, GLuint> mLocations;
        Dazzle::RenderSystem::GL::ProgramObject mProgram;
    };

    struct Material
    {
        glm::vec3 mAmbient;
        glm::vec3 mDiffuse;
        glm::vec3 mSpecular;
        float mShininess;
    };

    SceneToneMapping() :  mMVP(), mModelView(), mNormalMtx(),
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

        mHDRConfig.mWidth = mWidth;
        mHDRConfig.mHeight = mHeight;
        mHDRConfig.mMipmapLevel = 0;
        mHDRConfig.mUseDepthComponent = true;
        CreateTexture(mHDRConfig.mTexture, mWidth, mHeight);

        mHDRFrameBuffer = CreateFramebufferObject(mHDRConfig);

        // -----------------------------------------------------------------------------------------
        // 3D Objects for this scene:
        // Full Screen Quad
        mQuad = std::make_unique<Dazzle::Plane>(2.0f, 2.0f, 1, 1);
        mQuad->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        mQuad->Rotate(glm::vec3(1.0f, 0.0f, 0.0f), 90.0f);
        mQuad->InitializeBuffers();

        mFloor = std::make_unique<Dazzle::Plane>(20.0f, 10.0f, 1, 1);
        mFloor->SetPosition(glm::vec3(0.0f, -5.0f, 0.0f));
        mFloor->InitializeBuffers();

        mCeiling = std::make_unique<Dazzle::Plane>(20.0f, 10.0f, 1, 1);
        mCeiling->SetPosition(glm::vec3(0.0f, 5.0f, 0.0f));
        mCeiling->Rotate(glm::vec3(1.0f, 0.0f, 0.0f), 180.0f);
        mCeiling->InitializeBuffers();

        mWall = std::make_unique<Dazzle::Plane>(20.0f, 10.0f, 1, 1);
        mWall->Rotate(glm::vec3(1.0f, 0.0f, 0.0f), 90.0f);
        mWall->InitializeBuffers();

        mSphere = std::make_unique<Dazzle::Sphere>(1.5f, 64, 64, 0.0f, glm::pi<float>(), 0.0f, glm::two_pi<float>());
        mSphere->SetPosition(glm::vec3(-2.0f, -3.0f, 2.0f));
        mSphere->InitializeBuffers();

        mTorus = std::make_unique<Dazzle::Torus>(1.2f, 0.6f, 64, 64, 0.0f, glm::two_pi<float>(), 0.0f, glm::two_pi<float>());
        mTorus->SetPosition(glm::vec3(2.0f, -3.0f, 2.0f));
        mTorus->InitializeBuffers();

        // -----------------------------------------------------------------------------------------
        // Textures for this scene:
        // Bind Texture Unit 0 to Texture Object
        glBindTextureUnit(0, mHDRConfig.mTexture);

        // -----------------------------------------------------------------------------------------
        // Shader Program
        InitializeShaderProgram();
        InitializeUniforms();
    }

    void Update(double time) override
    {

    }

    void Render() override
    {
        Pass1(); // Render Scene to Texture
        CalculateLogarithmicAverageLuminance();
        Pass2(); // Apply Tone Mapping and Render Scene
    }

    void SetExposure(float exposure)
    {
        glUniform1f(mShader.mLocations.at("Exposure"), exposure);
    }

    void SetWhitePoint(float whitePoint)
    {
        glUniform1f(mShader.mLocations.at("WhitePoint"), whitePoint);
    }

    void EnableToneMapping(bool enable)
    {
        glUniform1i(mShader.mLocations.at("EnableToneMapping"), enable ? 1 : 0);
    }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override
    {
        mWidth = width; mHeight = height;
        glViewport(0, 0, mWidth, mHeight);
    }

private:

    // Pass #1:
    void Pass1()
    {
        // Shader Pass #1
        glUniform1i(mShader.mLocations.at("Pass"), 1);

        glBindFramebuffer(GL_FRAMEBUFFER, mHDRFrameBuffer);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        UpdateUniforms(mCamera->GetTransform());

        UpdateMaterial(mWallMaterial);
        UpdateMatrices(mShader, mFloor->GetTransform());
        mFloor->Draw();
        UpdateMatrices(mShader, mWall->GetTransform());
        mWall->Draw();
        UpdateMatrices(mShader, mCeiling->GetTransform());
        mCeiling->Draw();

        UpdateMaterial(mSphereMaterial);
        UpdateMatrices(mShader, mSphere->GetTransform());
        mSphere->Draw();

        UpdateMaterial(mTorusMaterial);
        UpdateMatrices(mShader, mTorus->GetTransform());
        mTorus->Draw();
    }

    // Calculate the logarithmic average luminance from an HDR texture
    void CalculateLogarithmicAverageLuminance()
    {
        int size = mWidth * mHeight; // Total number of pixels in the texture
        std::vector<float> textureData(size * 3); // RGB data for each pixel
        // Bind the HDR texture and read its pixel data
        // Note: This assumes the texture is in RGB format with floating-point values
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mHDRConfig.mTexture);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, textureData.data());
        float sum = 0.0f;
        for (int i = 0; i < size * 3; i += 3)
        {
            float r = textureData[i];
            float g = textureData[i + 1];
            float b = textureData[i + 2];
            // Calculate luminance using the Rec. 709 formula
            float luminance = glm::dot(glm::vec3(r, g, b), glm::vec3(0.2126f, 0.7152f, 0.0722f));
            sum += logf(luminance + 0.00001f); // Avoid log(0)
        }

        glUniform1f(mShader.mLocations.at("AverageLuminance"), expf(sum / size));
    }

    // Pass #2:
    void Pass2()
    {
        // Shader Pass #2
        glUniform1i(mShader.mLocations.at("Pass"), 2);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    void UpdateUniforms(const glm::mat4& transform)
    {
        glm::vec4 lightPosition;
        lightPosition = transform * glm::vec4(-7.0f, 4.0f, 2.5f, 1.0f);
        glUniform4fv(mShader.mLocations.at("lights[0].mPosition"), 1, glm::value_ptr(lightPosition));

        lightPosition = transform * glm::vec4(0.0f, 4.0f, 2.5f, 1.0f);
        glUniform4fv(mShader.mLocations.at("lights[1].mPosition"), 1, glm::value_ptr(lightPosition));

        lightPosition = transform * glm::vec4(7.0f, 4.0f, 2.5f, 1.0f);
        glUniform4fv(mShader.mLocations.at("lights[2].mPosition"), 1, glm::value_ptr(lightPosition));
    }

    void UpdateMaterial(const Material& material)
    {
        glUniform3fv(mShader.mLocations.at("material.mAmbient"), 1, glm::value_ptr(material.mAmbient));
        glUniform3fv(mShader.mLocations.at("material.mDiffuse"), 1, glm::value_ptr(material.mDiffuse));
        glUniform3fv(mShader.mLocations.at("material.mSpecular"), 1, glm::value_ptr(material.mSpecular));
        glUniform1f(mShader.mLocations.at("material.mShininess"), material.mShininess);
    }

    void InitializeUniforms()
    {
        // Lights
        glUniform3fv(mShader.mLocations.at("lights[0].mAmbient"), 1, glm::value_ptr(glm::vec3(0.2f)));
        glUniform3fv(mShader.mLocations.at("lights[0].mDiffuseSpecular"), 1, glm::value_ptr(glm::vec3(1.0f)));
        glUniform4fv(mShader.mLocations.at("lights[0].mPosition"), 1, glm::value_ptr(glm::vec4(-7.0f, 4.0f, 2.5f, 1.0f)));

        glUniform3fv(mShader.mLocations.at("lights[1].mAmbient"), 1, glm::value_ptr(glm::vec3(0.2f)));
        glUniform3fv(mShader.mLocations.at("lights[1].mDiffuseSpecular"), 1, glm::value_ptr(glm::vec3(1.0f)));
        glUniform4fv(mShader.mLocations.at("lights[1].mPosition"), 1, glm::value_ptr(glm::vec4(0.0f, 4.0f, 2.5f, 1.0f)));

        glUniform3fv(mShader.mLocations.at("lights[2].mAmbient"), 1, glm::value_ptr(glm::vec3(0.2f)));
        glUniform3fv(mShader.mLocations.at("lights[2].mDiffuseSpecular"), 1, glm::value_ptr(glm::vec3(1.0f)));
        glUniform4fv(mShader.mLocations.at("lights[2].mPosition"), 1, glm::value_ptr(glm::vec4(7.0f, 4.0f, 2.5f, 1.0f)));

        // Materials
        // Walls Material
        mWallMaterial.mAmbient = glm::vec3(0.2f, 0.2f, 0.2f);
        mWallMaterial.mDiffuse = glm::vec3(0.9f, 0.3f, 0.2f);
        mWallMaterial.mSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
        mWallMaterial.mShininess = 100.0f;
        // Sphere Material
        mSphereMaterial.mAmbient = glm::vec3(0.2f, 0.2f, 0.2f);
        mSphereMaterial.mDiffuse = glm::vec3(0.4f, 0.9f, 0.4f);
        mSphereMaterial.mSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
        mSphereMaterial.mShininess = 100.0f;
        // Torus Material
        mTorusMaterial.mAmbient = glm::vec3(0.2f, 0.2f, 0.2f);
        mTorusMaterial.mDiffuse = glm::vec3(0.4f, 0.4f, 0.9f);
        mTorusMaterial.mSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
        mTorusMaterial.mShininess = 100.0f;
    }

    void InitializeShaderProgram()
    {
        // Get the source code for shaders
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\ToneMapping.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\ToneMapping.fs.glsl");

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

        mShader.mLocations["lights[0].mAmbient"] = glGetUniformLocation(handle, "lights[0].mAmbient");
        mShader.mLocations["lights[0].mDiffuseSpecular"] = glGetUniformLocation(handle, "lights[0].mDiffuseSpecular");
        mShader.mLocations["lights[0].mPosition"] = glGetUniformLocation(handle, "lights[0].mPosition");

        mShader.mLocations["lights[1].mAmbient"] = glGetUniformLocation(handle, "lights[1].mAmbient");
        mShader.mLocations["lights[1].mDiffuseSpecular"] = glGetUniformLocation(handle, "lights[1].mDiffuseSpecular");
        mShader.mLocations["lights[1].mPosition"] = glGetUniformLocation(handle, "lights[1].mPosition");

        mShader.mLocations["lights[2].mAmbient"] = glGetUniformLocation(handle, "lights[2].mAmbient");
        mShader.mLocations["lights[2].mDiffuseSpecular"] = glGetUniformLocation(handle, "lights[2].mDiffuseSpecular");
        mShader.mLocations["lights[2].mPosition"] = glGetUniformLocation(handle, "lights[2].mPosition");

        mShader.mLocations["material.mAmbient"] = glGetUniformLocation(handle, "material.mAmbient");
        mShader.mLocations["material.mDiffuse"] = glGetUniformLocation(handle, "material.mDiffuse");
        mShader.mLocations["material.mSpecular"] = glGetUniformLocation(handle, "material.mSpecular");
        mShader.mLocations["material.mShininess"] = glGetUniformLocation(handle, "material.mShininess");

        mShader.mLocations["Pass"] = glGetUniformLocation(handle, "Pass");
        mShader.mLocations["AverageLuminance"] = glGetUniformLocation(handle, "AverageLuminance");
        mShader.mLocations["EnableToneMapping"] = glGetUniformLocation(handle, "EnableToneMapping");
        mShader.mLocations["Exposure"] = glGetUniformLocation(handle, "Exposure");
        mShader.mLocations["WhitePoint"] = glGetUniformLocation(handle, "WhitePoint");

        glUseProgram(mShader.mProgram.GetHandle());
    }

    struct FramebufferConfig
    {
        GLuint mTexture;
        GLsizei mWidth;
        GLsizei mHeight;
        GLint mMipmapLevel;
        bool mUseDepthComponent;
    };

    void CreateTexture(GLuint& texture, GLsizei width, GLsizei height)
    {
        // Create Texture Object
        glCreateTextures(GL_TEXTURE_2D, 1, &texture);
        glTextureStorage2D(texture, 1, GL_RGB32F, width, height);
        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    GLuint CreateFramebufferObject(const FramebufferConfig& config)
    {
        // Create Framebuffer Object
        GLuint fbo;
        glCreateFramebuffers(1, &fbo);

        // Bind Texture to FBO
        glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, config.mTexture, config.mMipmapLevel);

        if (config.mUseDepthComponent)
        {
            // Create Depth Buffer
            GLuint depthBuffer;
            glCreateRenderbuffers(1, &depthBuffer);
            glNamedRenderbufferStorage(depthBuffer, GL_DEPTH_COMPONENT, config.mWidth, config.mHeight);

            // Bind Depth Buffer to FBO
            glNamedFramebufferRenderbuffer(fbo, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
        }

        // Set targets for the fragment shader output variables
        GLenum drawBuffers[] = {GL_NONE, GL_COLOR_ATTACHMENT0};
        glNamedFramebufferDrawBuffers(fbo, 2, drawBuffers);

        // Check the framebuffer completeness status
        GLenum result = glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER);
        if (result != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "Framebuffer Error: " << result << '\n';

        return fbo;
    }

    FramebufferConfig mHDRConfig;
    GLuint mHDRFrameBuffer;
    ShaderProgram mShader;

    std::unique_ptr<Dazzle::Plane> mQuad;
    std::unique_ptr<Dazzle::Plane> mWall;
    std::unique_ptr<Dazzle::Plane> mFloor;
    std::unique_ptr<Dazzle::Plane> mCeiling;
    std::unique_ptr<Dazzle::Sphere> mSphere;
    std::unique_ptr<Dazzle::Torus> mTorus;
    std::shared_ptr<Camera> mCamera;

    Material mWallMaterial;
    Material mSphereMaterial;
    Material mTorusMaterial;

    int mWidth; // Framebuffer Width
    int mHeight; // Framebuffer Height

    glm::mat4 mMVP;
    glm::mat4 mModelView;
    glm::mat3 mNormalMtx;
};

class UIToneMapping : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneToneMapping*>(scene); }
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
        ImGui::SeparatorText("Tone Mapping");

        ImGui::NewLine();
        ImGui::Text("Toggle Tone Mapping:"); ImGui::SameLine(); ImGui::Checkbox("##Enable Tone Mapping", &mEnableToneMapping);
        ImGui::Text("Exposure:"); ImGui::SameLine(); ImGui::DragFloat("##Exposure", &mExposure, 0.01f, 0.01f, 2.0f, "%.2f");
        ImGui::Text("White Point:"); ImGui::SameLine(); ImGui::DragFloat("##White Point", &mWhitePoint, 0.01f, 0.01f, 2.0f, "%.3f");

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
            mScene->SetExposure(mExposure);
            mScene->SetWhitePoint(mWhitePoint);
            mScene->EnableToneMapping(mEnableToneMapping);
        }
    }

private:
    void ResetToDefaults()
    {
        mEnableToneMapping = true;
        mExposure = 0.35f;
        mWhitePoint = 0.928f;
    }

    SceneToneMapping* mScene = nullptr;
    Camera* mCamera = nullptr;

    bool mEnableToneMapping = true;
    float mExposure = 0.35f;
    float mWhitePoint = 0.928f;
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = WINDOW_WIDTH; // Window Width
    config.height = WINDOW_HEIGHT; // Window Height
    config.title = "Tone Mapping";    // Window Title

    auto sceneToneMapping = std::make_unique<SceneToneMapping>();
    auto uiToneMapping = std::make_unique<UIToneMapping>();

    App app(config, std::move(sceneToneMapping), std::move(uiToneMapping));
    app.Run();

    return 0;
}
