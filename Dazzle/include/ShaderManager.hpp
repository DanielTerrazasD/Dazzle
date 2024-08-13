#ifndef _SHADER_MANAGER_HPP_
#define _SHADER_MANAGER_HPP_

#include <GL/gl3w.h>

#include "DazzleEngine.hpp"
#include "Singleton.hpp"

namespace Dazzle
{
    namespace GL
    {
        class VAO
        {
        public:
            VAO();
            VAO(const VAO& other);
            VAO(VAO&& other) noexcept;
            ~VAO();

            VAO& operator=(const VAO& program);
            VAO& operator=(VAO&& program) noexcept;

            GLuint GetHandle() const;
            bool IsValid() const;
        private:
            GLuint mHandle;
        };

        class ShaderId
        {
        public:
            ShaderId();
            ShaderId(const ShaderId& other);
            ShaderId(ShaderId&& other) noexcept;
            ~ShaderId();

            ShaderId& operator=(const ShaderId& other);
            ShaderId& operator=(ShaderId&& other) noexcept;

            GLuint GetHandle() const;
            bool IsValid() const;
        private:
            friend class ShaderBuilder;
            GLuint mHandle;
        };

        class ShaderBuilder
        {
        public:
            ShaderBuilder();
            ~ShaderBuilder();
            void Create(const GLenum& type, const std::string& source, ShaderId& res);
        };

        class ProgramId
        {
        public:
            ProgramId();
            ProgramId(const ProgramId& other);
            ProgramId(ProgramId&& other) noexcept;
            ~ProgramId();

            ProgramId& operator=(const ProgramId& program);
            ProgramId& operator=(ProgramId&& program) noexcept;

            GLuint GetHandle() const;
            bool IsValid() const;
        private:
            friend class ProgramBuilder;
            GLuint mHandle;
        };

        class ProgramBuilder
        {
        public:
            ProgramBuilder();
            ~ProgramBuilder();
            void Create(ProgramId& program);
            void AttachShader(const ShaderId& shader, const ProgramId& program);
            void Link(const ProgramId& program);
        };
    }

    struct ShaderSources
    {
        std::string mVSSC;  // Vertex Shader Source Code
        std::string mTCSSC; // Tessellation Control Shader Source Code
        std::string mTESSC; // Tessellation Evaluation Shader Source Code
        std::string mGSSC;  // Geometry Shader Source Code
        std::string mFSSC;  // Fragment Shader Source Code
        std::string mCSSC;  // Compute Shader Source Code

        void SetVertexShader(const std::string& source) { mVSSC = source; }
        void SetTessControlShader(const std::string& source) { mTCSSC = source; }
        void SetTessEvaluationShader(const std::string& source) { mTESSC = source; }
        void SetGeometryShader(const std::string& source) { mGSSC = source; }
        void SetFragmentShader(const std::string& source) { mFSSC = source; }
        void SetComputeShader(const std::string& source) { mCSSC = source; }
    };

    // ShaderEffects
    // {
    //     // Basic Materials
    //     kSimpleShader,
    //     kStandardShader,
    //     kFlatColorShader,

    //     // Lighting Effects
    //     kDiffuseLighting,
    //     kSpecularLighting,
    //     kAmbientOcclusion,
    //     kReflectionShader,

    //     // Post-Processing Effects
    //     kBloom,
    //     kMotionBlur,
    //     kDepthOfField,
    //     kColorGrading,
    //     kEdgeDetection,

    //     // Texturing and Surface
    //     kTextureMapping,
    //     kNormalMapping,
    //     kParallaxMapping,
    //     kBumpMapping,
    //     kProceduralTextures,

    //     // Visual Effects
    //     kGlow,
    //     kOutline,
    //     kWaterShader,
    //     kFireShader,
    //     kIceShader,

    //     // Distortion Effects
    //     kWaveDistortion,
    //     kRefraction,
    //     kRippleEffect,
    //     kHeatHaze,

    //     // UI Effects
    //     kButtonHighlight,
    //     kPanelGlow,
    //     kTextShadow,
    //     kFadeTransition,

    //     // Environment Effects
    //     kFog,
    //     kRainShader,
    //     kSnowShader,
    //     kCloudShader,

    //     // Special Effects
    //     kMagicEffect,
    //     kPortalEffect,
    //     kEnergyField,
    //     kParticleShader,

    //     // Debugging and Development
    //     kDebugNormals,
    //     kWireframeView,
    //     kUVGrid
    // };

    class ShaderObject
    {
    public:
        virtual ~ShaderObject() = 0;
        virtual void Use() const = 0;
    };

    class SimpleShader : public ShaderObject
    {
    public:
        SimpleShader();
        ~SimpleShader() override;
        virtual void Use() const override;

    private:
        GL::VAO mVAO;
        GL::ProgramId mProgram;
    };

    class ShaderManager
    {
    public:
        ~ShaderManager();
        void UseShader(const ShaderObject& shader);

        ShaderObject& GetSimpleShader() { return mSimpleShader.GetInstance(); }

    private:
        Singleton<ShaderObject, SimpleShader> mSimpleShader;
    };
}

#endif // _SHADER_MANAGER_HPP_