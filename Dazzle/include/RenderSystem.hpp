#ifndef _RENDER_SYSTEM_HPP_
#define _RENDER_SYSTEM_HPP_

#include <string>
#include <unordered_map>
#include <vector>

#include <GL/gl3w.h>

#include "DazzleEngine.hpp"

namespace Dazzle
{
    namespace RenderSystem
    {
    }

    namespace GL
    {
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
        
        void SetupDebugMessageCallback();
        void DebugMessageCallback(   GLenum source,
                            GLenum type,
                            GLuint id,
                            GLenum severity,
                            GLsizei length,
                            const GLchar *message,
                            const void *userParam);

    }

    struct FileLoader
    {
        static std::string ReadFile(const std::string& path);
    };

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

    enum class ShaderEffect;
    class ShaderManager
    {
    public:
        void LoadShader(const ShaderSources& sources, const ShaderEffect& effect);
        void UseShader(const ShaderEffect& effect);
    private:
        std::unordered_map<ShaderEffect, GL::ProgramId> mShaders;
    };

    enum class ShaderEffect
    {
        // Basic Materials
        kSimpleShader,
        kStandardShader,
        kFlatColorShader,

        // Lighting Effects
        kDiffuseLighting,
        kSpecularLighting,
        kAmbientOcclusion,
        kReflectionShader,

        // Post-Processing Effects
        kBloom,
        kMotionBlur,
        kDepthOfField,
        kColorGrading,
        kEdgeDetection,

        // Texturing and Surface
        kTextureMapping,
        kNormalMapping,
        kParallaxMapping,
        kBumpMapping,
        kProceduralTextures,

        // Visual Effects
        kGlow,
        kOutline,
        kWaterShader,
        kFireShader,
        kIceShader,

        // Distortion Effects
        kWaveDistortion,
        kRefraction,
        kRippleEffect,
        kHeatHaze,

        // UI Effects
        kButtonHighlight,
        kPanelGlow,
        kTextShadow,
        kFadeTransition,

        // Environment Effects
        kFog,
        kRainShader,
        kSnowShader,
        kCloudShader,

        // Special Effects
        kMagicEffect,
        kPortalEffect,
        kEnergyField,
        kParticleShader,

        // Debugging and Development
        kDebugNormals,
        kWireframeView,
        kUVGrid
    };
}
#endif // _RENDER_SYSTEM_HPP_
