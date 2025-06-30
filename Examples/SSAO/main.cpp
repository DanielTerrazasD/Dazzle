#include <array>
#include <iostream>
#include <random>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"

#include "RenderSystem.hpp"
#include "FileManager.hpp"
#include "Mesh.hpp"
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

class RandomHelper
{
public:
    RandomHelper() : mDistribution(0.0f, 1.0f)
    {
        std::random_device rd;
        mGenerator.seed(rd());
    }

    float GetRandomFloat()
    {
        return mDistribution(mGenerator);
    }

    glm::vec3 UniformCircularSample()
    {
        float angle = GetRandomFloat() * glm::two_pi<float>();
        return glm::vec3(glm::cos(angle), sin(angle), 0.0f);
    }

    glm::vec3 UniformHemisphereSample()
    {
        glm::vec3 result;
        float u = GetRandomFloat();
        float v = GetRandomFloat();
        float scale = sqrt(1.0f - u * u);
        result.x = glm::cos(glm::two_pi<float>() * v) * scale;
        result.y = glm::sin(glm::two_pi<float>() * v) * scale;
        result.z = u; // z is the height in the hemisphere
        return result;
    }

private:
    std::mt19937 mGenerator;
    std::uniform_real_distribution<float> mDistribution;
};

class SceneSSAO : public IScene
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
        bool mUseTexture;
    };

    struct FramebufferConfig
    {
        GLuint mTexture;
        GLsizei mWidth;
        GLsizei mHeight;
        GLint mMipmapLevel;
        bool mUseDepthComponent;
    };

    SceneSSAO() :  mMVP(), mModelView(), mNormalMtx(),
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

        mSuzanne = std::make_unique<Dazzle::Mesh>();
        mSuzanne->SetPosition(glm::vec3(0.0f, -1.0f, -5.0f));
        InitializeMesh(*mSuzanne, "models\\suzanne.obj");
        mSuzanne->InitializeBuffers();

        mFloor = std::make_unique<Dazzle::Plane>(5.0f, 5.0f, 1, 1);
        mFloor->SetPosition(glm::vec3(0.0f, -2.5f, -5.0f));
        mFloor->InitializeBuffers();

        mLeftWall = std::make_unique<Dazzle::Plane>(5.0f, 5.0f, 1, 1);
        mLeftWall->Translate(glm::vec3(-2.5f, 0.0f, -5.0f));
        mLeftWall->Rotate(glm::vec3(1.0f, 0.0f, 0.0f), 90.0f);
        mLeftWall->Rotate(glm::vec3(0.0f, 0.0f, 1.0f), -90.0f);
        mLeftWall->InitializeBuffers();

        mRightWall = std::make_unique<Dazzle::Plane>(5.0f, 5.0f, 1, 1);
        mRightWall->Translate(glm::vec3(0.0f, 0.0f, -7.5f));
        mRightWall->Rotate(glm::vec3(1.0f, 0.0f, 0.0f), 90.0f);
        mRightWall->InitializeBuffers();

        // -----------------------------------------------------------------------------------------
        // Textures:
        mWoodTexture = LoadTexture("textures\\hardwood.jpg");
        mBrickTexture = LoadTexture("textures\\brick.jpg");
        mRandomTexture = CreateRandomRotationTexture();

        CreateTexture(mPositionTexture, GL_RGB32F, mWidth, mHeight);
        CreateTexture(mNormalTexture, GL_RGB32F, mWidth, mHeight);
        CreateTexture(mColorTexture, GL_RGB8, mWidth, mHeight);
        CreateTexture(mAOTexture[0], GL_R16F, mWidth, mHeight);
        CreateTexture(mAOTexture[1], GL_R16F, mWidth, mHeight);

        // Bind Texture Units
        glBindTextureUnit(0, mPositionTexture); // Position Texture
        glBindTextureUnit(1, mNormalTexture); // Normal Texture
        glBindTextureUnit(2, mColorTexture); // Color Texture
        glBindTextureUnit(3, mAOTexture[0]); // Ambient Occlusion Texture
        glBindTextureUnit(4, mRandomTexture); // Random Texture
        glBindTextureUnit(5, mWoodTexture); // Diffuse Texture

        // -----------------------------------------------------------------------------------------
        // Framebuffer Objects:

        // Deferred Rendering FBO
        const GLint kMipmapLevel = 0;
        GLuint depthBuffer;
        glCreateFramebuffers(1, &mDeferredFBO);
        glNamedFramebufferTexture(mDeferredFBO, GL_COLOR_ATTACHMENT0, mPositionTexture, kMipmapLevel);
        glNamedFramebufferTexture(mDeferredFBO, GL_COLOR_ATTACHMENT1, mNormalTexture, kMipmapLevel);
        glNamedFramebufferTexture(mDeferredFBO, GL_COLOR_ATTACHMENT2, mColorTexture, kMipmapLevel);
        glCreateRenderbuffers(1, &depthBuffer);
        glNamedRenderbufferStorage(depthBuffer, GL_DEPTH_COMPONENT, mWidth, mHeight);
        glNamedFramebufferRenderbuffer(mDeferredFBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
        GLenum deferredDrawBuffers[] = {GL_NONE, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_NONE};
        glNamedFramebufferDrawBuffers(mDeferredFBO, 5, deferredDrawBuffers);
        CheckFBOCompleteness(mDeferredFBO);

        // SSAO FBO
        glCreateFramebuffers(1, &mSSAOFBO);
        glNamedFramebufferTexture(mSSAOFBO, GL_COLOR_ATTACHMENT0, mAOTexture[0], kMipmapLevel);
        GLenum SSAODrawBuffers[] = {GL_NONE, GL_NONE, GL_NONE, GL_NONE, GL_COLOR_ATTACHMENT0};
        glNamedFramebufferDrawBuffers(mSSAOFBO, 5, SSAODrawBuffers);
        CheckFBOCompleteness(mSSAOFBO);

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
        Pass1(); // Render Scene
        Pass2(); // SSAO Pass
        Pass3(); // Blur Pass
        Pass4(); // Lighting Pass
    }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override
    {
        mWidth = width; mHeight = height;
        glViewport(0, 0, mWidth, mHeight);
    }

    void SetRadius(float radius)
    {
        glUniform1f(mShader.mLocations.at("Radius"), radius);
    }

private:

    void DrawScene()
    {
        UpdateUniforms(mCamera->GetTransform(), mCamera->GetProjection());

        // Draw Floor
        glBindTextureUnit(5, mWoodTexture);
        UpdateMaterial(mWallsMaterial);
        UpdateMatrices(mShader, mFloor->GetTransform());
        mFloor->Draw();

        glBindTextureUnit(5, mBrickTexture);
        UpdateMatrices(mShader, mLeftWall->GetTransform());
        mLeftWall->Draw();
        UpdateMatrices(mShader, mRightWall->GetTransform());
        mRightWall->Draw();

        UpdateMaterial(mSuzanneMaterial);
        UpdateMatrices(mShader, mSuzanne->GetTransform());
        mSuzanne->Draw();
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

    // Pass #1: Render Scene to G-Buffers
    void Pass1()
    {
        // Shader Pass #1
        glUniform1i(mShader.mLocations.at("Pass"), 1);

        glBindFramebuffer(GL_FRAMEBUFFER, mDeferredFBO);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        DrawScene();
    }

    // Pass #2: Calculate Screen Space Ambient Occlusion
    void Pass2()
    {
        // Shader Pass #2
        glUniform1i(mShader.mLocations.at("Pass"), 2);

        glBindFramebuffer(GL_FRAMEBUFFER, mSSAOFBO);

        // Write to mAOTexture[0]
        glBindTextureUnit(3, mAOTexture[1]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mAOTexture[0], 0);

        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        DrawQuad();
    }

    // Pass #3: Blur Ambient Occlusion Texture
    void Pass3()
    {
        // Shader Pass #3
        glUniform1i(mShader.mLocations.at("Pass"), 3);

        // Read from mAOTexture[0] and write to mAOTexture[1]
        glBindTextureUnit(3, mAOTexture[0]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mAOTexture[1], 0);

        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        DrawQuad();
    }

    // Pass #4: Lighting Pass and apply Ambient Occlusion
    void Pass4()
    {
        // Shader Pass #4
        glUniform1i(mShader.mLocations.at("Pass"), 4);

        // Read from mAOTexture[1] blurred
        glBindTextureUnit(3, mAOTexture[1]);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind to default framebuffer

        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

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

    void UpdateUniforms(const glm::mat4& viewTransform, const glm::mat4& viewProjection)
    {
        glm::vec4 lightPosition;
        lightPosition = viewTransform * glm::vec4(3.0f, 3.0f, 1.5f, 1.0f);
        glUniform4fv(mShader.mLocations.at("light.mPosition"), 1, glm::value_ptr(lightPosition));

        glUniformMatrix4fv(mShader.mLocations.at("ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(viewProjection));
    }

    void UpdateMaterial(const Material& material)
    {
        glUniform3fv(mShader.mLocations.at("material.mAmbient"), 1, glm::value_ptr(material.mAmbient));
        glUniform3fv(mShader.mLocations.at("material.mDiffuse"), 1, glm::value_ptr(material.mDiffuse));
        glUniform3fv(mShader.mLocations.at("material.mSpecular"), 1, glm::value_ptr(material.mSpecular));
        glUniform1f(mShader.mLocations.at("material.mShininess"), material.mShininess);
        glUniform1i(mShader.mLocations.at("material.mUseTexture"), material.mUseTexture);
    }

    void InitializeUniforms()
    {
        // Lights
        glUniform3fv(mShader.mLocations.at("light.mAmbient"), 1, glm::value_ptr(glm::vec3(0.2f)));
        glUniform3fv(mShader.mLocations.at("light.mDiffuseSpecular"), 1, glm::value_ptr(glm::vec3(1.0f)));
        glUniform4fv(mShader.mLocations.at("light.mPosition"), 1, glm::value_ptr(glm::vec4(-7.0f, 4.0f, 2.5f, 1.0f)));

        // Materials
        mSuzanneMaterial.mAmbient = glm::vec3(0.0f);
        mSuzanneMaterial.mDiffuse = glm::vec3(0.9f, 0.5f, 0.2f);
        mSuzanneMaterial.mSpecular = glm::vec3(0.0f);
        mSuzanneMaterial.mShininess = 0.0f;
        mSuzanneMaterial.mUseTexture = false;

        mWallsMaterial.mAmbient = glm::vec3(0.0f, 0.0f, 0.0f);
        mWallsMaterial.mDiffuse = glm::vec3(0.0f, 0.0f, 0.0f);
        mWallsMaterial.mSpecular = glm::vec3(0.0f, 0.0f, 0.0f);
        mWallsMaterial.mShininess = 0.0f;
        mWallsMaterial.mUseTexture = true;

        InitializeKernel();
    }

    void InitializeShaderProgram()
    {
        // Get the source code for shaders
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\SSAO.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\SSAO.fs.glsl");

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

        mShader.mLocations["light.mAmbient"] = glGetUniformLocation(handle, "light.mAmbient");
        mShader.mLocations["light.mDiffuseSpecular"] = glGetUniformLocation(handle, "light.mDiffuseSpecular");
        mShader.mLocations["light.mPosition"] = glGetUniformLocation(handle, "light.mPosition");

        mShader.mLocations["material.mAmbient"] = glGetUniformLocation(handle, "material.mAmbient");
        mShader.mLocations["material.mDiffuse"] = glGetUniformLocation(handle, "material.mDiffuse");
        mShader.mLocations["material.mSpecular"] = glGetUniformLocation(handle, "material.mSpecular");
        mShader.mLocations["material.mShininess"] = glGetUniformLocation(handle, "material.mShininess");
        mShader.mLocations["material.mUseTexture"] = glGetUniformLocation(handle, "material.mUseTexture");

        mShader.mLocations["Pass"] = glGetUniformLocation(handle, "Pass");
        mShader.mLocations["ProjectionMatrix"] = glGetUniformLocation(handle, "ProjectionMatrix");
        mShader.mLocations["SampleKernel"] = glGetUniformLocation(handle, "SampleKernel");
        mShader.mLocations["Radius"] = glGetUniformLocation(handle, "Radius");

        glUseProgram(mShader.mProgram.GetHandle());
    }

    void CreateTexture(GLuint& texture, const GLenum& format, GLsizei width, GLsizei height)
    {
        // Create Texture Object
        glCreateTextures(GL_TEXTURE_2D, 1, &texture);
        glTextureStorage2D(texture, 1, format, width, height);

        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    }

    GLuint LoadTexture(const std::string& path, bool flip = true)
    {
        int width, height;
        auto imageData = Utils::Texture::GetTextureData(path, width, height, flip);
        GLuint texture = 0; // OpenGL Texture Object
        if (imageData)
        {
            // Create texture
            glCreateTextures(GL_TEXTURE_2D, 1, &texture);

            // Set texture parameters
            glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            // Explicitly state the base and max levels
            glTextureParameteri(texture, GL_TEXTURE_BASE_LEVEL, 0);
            glTextureParameteri(texture, GL_TEXTURE_MAX_LEVEL, 0);

            // Set up texture storage
            glTextureStorage2D(texture, 1, GL_RGBA8, width, height);

            // Upload texture data
            glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, imageData.get());
        }

        return texture;
    }

    void CheckFBOCompleteness(GLuint fbo)
    {
        GLenum status = glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cerr << "Framebuffer Error: " << status << '\n';
        }
    }

    // Random Kernel:
    // Kernel of random samples in the positive-z hemisphere
    // The samples are uniformly distributed over the hemisphere
    void InitializeKernel()
    {
        const int kKernelSize = 64;
        std::vector<float> kernel(3 * kKernelSize);
        for (int i = 0; i < kKernelSize; ++i)
        {
            glm::vec3 sample = mRandomHelper.UniformHemisphereSample();
            float scale = static_cast<float>(i * i) / static_cast<float>(kKernelSize * kKernelSize);
            sample *= glm::mix(0.1f, 1.0f, scale);

            kernel[3 * i] = sample.x;
            kernel[3 * i + 1] = sample.y;
            kernel[3 * i + 2] = sample.z;
        }

        glUniform3fv(mShader.mLocations.at("SampleKernel"), kKernelSize, kernel.data());
    }

    // Random Rotation Texture:
    // Texture with random rotation vectors, each vector is a unit vector in the x-y plane.
    // The texture is used to apply a random rotation to the samples of the kernel in the AO shader step.
    GLuint CreateRandomRotationTexture()
    {
        const int kTextureSize = 4;
        std::vector<float> randomDirections(3 * kTextureSize * kTextureSize);
        for (int i = 0; i < kTextureSize * kTextureSize; ++i)
        {
            glm::vec3 sample = mRandomHelper.UniformCircularSample();
            randomDirections[3 * i] = sample.x;
            randomDirections[3 * i + 1] = sample.y;
            randomDirections[3 * i + 2] = sample.z;
        }

        GLuint texture;
        glCreateTextures(GL_TEXTURE_2D, 1, &texture);
        glTextureStorage2D(texture, 1, GL_RGB16F, kTextureSize, kTextureSize);
        glTextureSubImage2D(texture, 0, 0, 0, kTextureSize, kTextureSize, GL_RGBA, GL_FLOAT, randomDirections.data());
        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(texture, GL_TEXTURE_MAX_LEVEL, 0);
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

    // Shading Programs
    ShaderProgram mShader;

    // Framebuffer Objects
    GLuint mDeferredFBO; // Deferred Rendering Framebuffer
    GLuint mSSAOFBO; // Screen Space Ambient Occlusion Framebuffer

    // Textures
    GLuint mWoodTexture;
    GLuint mBrickTexture;
    GLuint mRandomTexture; // Random Rotation Texture for SSAO
    GLuint mAOTexture[2]; // Ambient Occlusion Textures
    // G-Buffer Textures
    GLuint mPositionTexture;
    GLuint mNormalTexture;
    GLuint mColorTexture;

    // 3D Objects
    std::unique_ptr<Dazzle::Plane> mQuad;
    std::unique_ptr<Dazzle::Plane> mFloor;
    std::unique_ptr<Dazzle::Plane> mLeftWall;
    std::unique_ptr<Dazzle::Plane> mRightWall;
    std::unique_ptr<Dazzle::Mesh> mSuzanne;
    std::shared_ptr<Camera> mCamera;

    // Materials
    Material mSuzanneMaterial;
    Material mWallsMaterial;

    int mWidth; // Framebuffer Width
    int mHeight; // Framebuffer Height

    RandomHelper mRandomHelper;

    glm::mat4 mMVP;
    glm::mat4 mModelView;
    glm::mat3 mNormalMtx;
};

class UISSAO : public IUserInterface
{
public:

    void SetScene(IScene* scene) override { mScene = static_cast<SceneSSAO*>(scene); }
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
        ImGui::SeparatorText("Screen Space Ambient Occlusion (SSAO)");

        ImGui::NewLine();
        ImGui::Text("Radius:"); ImGui::SameLine(); ImGui::DragFloat("##Radius", &mRadius, 0.01f, 0.01f, 1.0f, "%.2f");

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
            mScene->SetRadius(mRadius);
        }
    }

private:
    void ResetToDefaults()
    {
        mRadius = 0.55f;
    }

    SceneSSAO* mScene = nullptr;
    Camera* mCamera = nullptr;

    float mRadius = 0.55f;
};

int main(int argc, char const *argv[])
{
    AppConfig config;
    config.width = WINDOW_WIDTH; // Window Width
    config.height = WINDOW_HEIGHT; // Window Height
    config.title = "Screen Space Ambient Occlusion (SSAO)";    // Window Title

    auto sceneSSAO = std::make_unique<SceneSSAO>();
    auto uiSSAO = std::make_unique<UISSAO>();

    App app(config, std::move(sceneSSAO), std::move(uiSSAO));
    app.Run();

    return 0;
}
