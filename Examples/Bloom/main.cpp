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

        // HDR Framebuffer Object
        mHDRConfig.mWidth = mWidth;
        mHDRConfig.mHeight = mHeight;
        mHDRConfig.mMipmapLevel = 0;
        mHDRConfig.mUseDepthComponent = true;
        CreateTexture(mHDRConfig.mTexture, mWidth, mHeight);
        mHDRFrameBuffer = CreateFramebufferObject(mHDRConfig);

        // Blur Framebuffer Object
        mBlurConfig.mWidth = mWidth / 8;
        mBlurConfig.mHeight = mHeight / 8;
        mBlurConfig.mMipmapLevel = 0;
        mBlurConfig.mUseDepthComponent = false;
        CreateTexture(mBlurConfig.mTexture, mBlurConfig.mWidth, mBlurConfig.mHeight);
        mBlurFrameBuffer = CreateFramebufferObject(mBlurConfig);

        CreateTexture(mTexture1, mBlurConfig.mWidth, mBlurConfig.mHeight);
        CreateTexture(mTexture2, mBlurConfig.mWidth, mBlurConfig.mHeight);

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
        // Bind Texture Units 1 and  to Blur Texture Objects
        glBindTextureUnit(1, mTexture1);
        glBindTextureUnit(2, mTexture2);
        
        // -----------------------------------------------------------------------------------------
        // Shader Program
        InitializeShaderProgram();
        InitializeUniforms();
        InitializeSamplers();
    }

    void Update(double time) override
    {

    }

    void Render() override
    {
        Pass1(); // Render Scene to Texture
        CalculateLogarithmicAverageLuminance(); // Calculate average luminance from HDR texture
        Pass2(); // Bright-pass filter
        Pass3(); // First Gaussian Blur Pass
        Pass4(); // Second Gaussian Blur Pass
        Pass5(); // Render Scene with HDR Tone Mapping
    }

    void SetExposure(float exposure)
    {
        glUniform1f(mShader.mLocations.at("Exposure"), exposure);
    }

    void SetWhitePoint(float whitePoint)
    {
        glUniform1f(mShader.mLocations.at("WhitePoint"), whitePoint);
    }

    void SetLuminanceThreshold(float threshold)
    {
        glUniform1f(mShader.mLocations.at("LuminanceThreshold"), threshold);
    }

    void KeyCallback(int key, int scancode, int action, int mods) override {}
    void CursorCallback(double xPosition, double yPosition) override {}
    void FramebufferResizeCallback(int width, int height) override
    {
        mWidth = width; mHeight = height;
        glViewport(0, 0, mWidth, mHeight);
    }

private:

    // Calculate the logarithmic average luminance from an HDR texture
    void CalculateLogarithmicAverageLuminance()
    {
        int size = mWidth * mHeight; // Total number of pixels in the texture
        std::vector<float> textureData(size * 3); // RGB data for each pixel
        // Bind the HDR texture and read its pixel data
        // Note: This assumes the texture is in RGB format with floating-point values
        glGetTextureImage(mHDRConfig.mTexture, 0, GL_RGB, GL_FLOAT, textureData.size() * sizeof(float), textureData.data());
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

    // Pass #1: Render the scene to an HDR texture
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

    // Pass #2: Apply a bright-pass filter to the HDR texture
    void Pass2()
    {
        // Shader Pass #2
        glUniform1i(mShader.mLocations.at("Pass"), 2);

        glBindFramebuffer(GL_FRAMEBUFFER, mBlurFrameBuffer);
        // Read from HDR texture and store the result in mTexture1
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture1, 0);
        glViewport(0, 0, mBlurConfig.mWidth, mBlurConfig.mHeight);
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

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

    // Pass #3: First Gaussian Blur Pass across the vertical axis
    void Pass3()
    {
        // Shader Pass #3
        glUniform1i(mShader.mLocations.at("Pass"), 3);
        // Read from mTexture1 and store the blurred texture in mTexture2
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture2, 0);
        // Draw
        mQuad->Draw();
    }

    // Pass #4: Second Gaussian Blur Pass across the horizontal axis
    void Pass4()
    {
        // Shader Pass #4
        glUniform1i(mShader.mLocations.at("Pass"), 4);
        // Read from mTexture2 and store the blurred texture in mTexture1
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture1, 0);
        // Draw
        mQuad->Draw();
    }

    // Pass #5: Render the final scene with HDR Tone Mapping and Bloom effect
    void Pass5()
    {
        // Shader Pass #5
        glUniform1i(mShader.mLocations.at("Pass"), 5);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mWidth, mHeight);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindSampler(1, mLinearSampler);
        
        // Draw
        mQuad->Draw();

        glBindSampler(1, mNearestSampler);
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

    void UpdateWeights(float newSigmaSquared)
    {
        if (std::fabs(mSigmaSquared - newSigmaSquared) <= 1e-6f)
            return;

        mSigmaSquared = newSigmaSquared;
        mWeights[0] = GaussKernel(0, mSigmaSquared);
        float sum = mWeights[0];
        for (size_t i = 1; i < mWeights.size(); ++i)
        {
            mWeights[i] = GaussKernel((float)i, mSigmaSquared);
            sum += 2 * mWeights[i];
        }

        for (size_t i = 0; i < mWeights.size(); ++i)
            mWeights[i] /= sum;

        glUniform1f(mShader.mLocations.at("Weight[0]"), mWeights[0]);
        glUniform1f(mShader.mLocations.at("Weight[1]"), mWeights[1]);
        glUniform1f(mShader.mLocations.at("Weight[2]"), mWeights[2]);
        glUniform1f(mShader.mLocations.at("Weight[3]"), mWeights[3]);
        glUniform1f(mShader.mLocations.at("Weight[4]"), mWeights[4]);
        glUniform1f(mShader.mLocations.at("Weight[5]"), mWeights[5]);
        glUniform1f(mShader.mLocations.at("Weight[6]"), mWeights[6]);
        glUniform1f(mShader.mLocations.at("Weight[7]"), mWeights[7]);
        glUniform1f(mShader.mLocations.at("Weight[8]"), mWeights[8]);
        glUniform1f(mShader.mLocations.at("Weight[9]"), mWeights[9]);
    }

    // 2D Gaussian Kernel:
    // Formula: (1 / 2*PI*Sigma^2) * pow(e, - (x^2 + y^2) / 2*Sigma^2)
    // 1D Gaussian Kernel:
    float GaussKernel(float x, float sigma2)
    {
        double coeff = 1.0 / (glm::two_pi<double>() * sigma2);
        double expon = -(x * x) / (2.0 * sigma2);
        return (float) (coeff * exp(expon));
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

        // Initialize Blur Weights
        float sigmaSquared = 25.0f;
        UpdateWeights(sigmaSquared);

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
        auto VSSC = Dazzle::FileManager::ReadFile("shaders\\Bloom.vs.glsl");
        auto FSSC = Dazzle::FileManager::ReadFile("shaders\\Bloom.fs.glsl");

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
        mShader.mLocations["Exposure"] = glGetUniformLocation(handle, "Exposure");
        mShader.mLocations["WhitePoint"] = glGetUniformLocation(handle, "WhitePoint");
        mShader.mLocations["LuminanceThreshold"] = glGetUniformLocation(handle, "LuminanceThreshold");

        mShader.mLocations["Weight[0]"] = glGetUniformLocation(handle, "Weight[0]");
        mShader.mLocations["Weight[1]"] = glGetUniformLocation(handle, "Weight[1]");
        mShader.mLocations["Weight[2]"] = glGetUniformLocation(handle, "Weight[2]");
        mShader.mLocations["Weight[3]"] = glGetUniformLocation(handle, "Weight[3]");
        mShader.mLocations["Weight[4]"] = glGetUniformLocation(handle, "Weight[4]");
        mShader.mLocations["Weight[5]"] = glGetUniformLocation(handle, "Weight[5]");
        mShader.mLocations["Weight[6]"] = glGetUniformLocation(handle, "Weight[6]");
        mShader.mLocations["Weight[7]"] = glGetUniformLocation(handle, "Weight[7]");
        mShader.mLocations["Weight[8]"] = glGetUniformLocation(handle, "Weight[8]");
        mShader.mLocations["Weight[9]"] = glGetUniformLocation(handle, "Weight[9]");

        glUseProgram(mShader.mProgram.GetHandle());
    }

    void InitializeSamplers()
    {
        // Create Sampler Object for HDR Texture
        GLfloat border[] = {0.0f,0.0f,0.0f,0.0f};
        glCreateSamplers(1, &mLinearSampler);
        glSamplerParameteri(mLinearSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(mLinearSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(mLinearSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glSamplerParameteri(mLinearSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glSamplerParameterfv(mLinearSampler, GL_TEXTURE_BORDER_COLOR, border);

        glCreateSamplers(1, &mNearestSampler);
        glSamplerParameteri(mNearestSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(mNearestSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameteri(mNearestSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glSamplerParameteri(mNearestSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glSamplerParameterfv(mNearestSampler, GL_TEXTURE_BORDER_COLOR, border);

        // We want nearest sampling except for the last pass.
        glBindSampler(0, mNearestSampler);
        glBindSampler(1, mNearestSampler);
        glBindSampler(2, mNearestSampler);
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
        GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
        glNamedFramebufferDrawBuffers(fbo, 1, drawBuffers);

        // Check the framebuffer completeness status
        GLenum result = glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER);
        if (result != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "Framebuffer Error: " << result << '\n';

        return fbo;
    }

    FramebufferConfig mBlurConfig;
    GLuint mBlurFrameBuffer;
    FramebufferConfig mHDRConfig;
    GLuint mHDRFrameBuffer;
    ShaderProgram mShader;

    GLuint mTexture1;
    GLuint mTexture2;

    float mSigmaSquared;
    std::array<float, 10> mWeights;
    GLuint mLinearSampler;
    GLuint mNearestSampler;

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
        ImGui::Text("Exposure:"); ImGui::SameLine(); ImGui::DragFloat("##Exposure", &mExposure, 0.01f, 0.01f, 2.0f, "%.2f");
        ImGui::Text("White Point:"); ImGui::SameLine(); ImGui::DragFloat("##White Point", &mWhitePoint, 0.01f, 0.01f, 2.0f, "%.3f");
        ImGui::Text("Luminance Threshold:"); ImGui::SameLine(); ImGui::DragFloat("##Luminance Threshold", &mLuminanceThreshold, 0.01f, 0.01f, 2.0f, "%.3f");

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
            mScene->SetLuminanceThreshold(mLuminanceThreshold);
        }
    }

private:
    void ResetToDefaults()
    {
        mExposure = 0.35f;
        mWhitePoint = 0.928f;
        mLuminanceThreshold = 1.7f;
    }

    SceneToneMapping* mScene = nullptr;
    Camera* mCamera = nullptr;

    float mExposure = 0.35f;
    float mWhitePoint = 0.928f;
    float mLuminanceThreshold = 1.7f;
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
