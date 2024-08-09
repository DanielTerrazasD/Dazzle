#ifndef _RENDER_SYSTEM_HPP_
#define _RENDER_SYSTEM_HPP_

#include <string>
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
            ~ShaderId();
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
            ShaderId Create(const GLenum& type, const std::string& source);
        };

        class ProgramId
        {
        public:
            ProgramId();
            ~ProgramId();
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
            ProgramId Create();
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

    enum class ShaderType
    {
        VERTEX_SHADER =             1 << 0,
        TESS_CONTROL_SHADER =       1 << 1,
        TESS_EVALUATION_SHADER =    1 << 2,
        GEOMETRY_SHADER =           1 << 3,
        FRAGMENT_SHADER =           1 << 4,
        COMPUTE_SHADER =            1 << 5
    };

    inline ShaderType operator|(ShaderType lhs, ShaderType rhs)
    {
        return static_cast<ShaderType>(
            static_cast<std::underlying_type<ShaderType>::type>(lhs) |
            static_cast<std::underlying_type<ShaderType>::type>(rhs));
    }

    inline ShaderType operator&(ShaderType lhs, ShaderType rhs)
    {
        return static_cast<ShaderType>(
            static_cast<std::underlying_type<ShaderType>::type>(lhs) &
            static_cast<std::underlying_type<ShaderType>::type>(rhs));
    }

    inline bool hasType(ShaderType bitField, ShaderType type)
    {
        return (static_cast<std::underlying_type<ShaderType>::type>(bitField) &
                static_cast<std::underlying_type<ShaderType>::type>(type)) != 0;
    }

    struct ShaderSources
    {
        std::string mVSSC;  // Vertex Shader Source Code
        std::string mTCSSC; // Tessellation Control Shader Source Code
        std::string mTESSC; // Tessellation Evaluation Shader Source Code
        std::string mGSSC;  // Geometry Shader Source Code
        std::string mFSSC;  // Fragment Shader Source Code
        std::string mCSSC;  // Compute Shader Source Code
    };

    class ShaderManager
    {
    public:
        void LoadShader(const ShaderSources& sources);
        void UseShader(/*Select by shaderId or shaderName*/);
    private:
        // std::unordered_map<> mShaders;
    };
}
#endif // _RENDER_SYSTEM_HPP_
