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
        namespace GL
        {
            void SetupDebugMessageCallback();
            void DebugMessageCallback(   GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar *message,
                                const void *userParam);
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

            class ShaderObject
            {
            public:
                ShaderObject();
                ShaderObject(const ShaderObject& other);
                ShaderObject(ShaderObject&& other) noexcept;
                ~ShaderObject();

                ShaderObject& operator=(const ShaderObject& other);
                ShaderObject& operator=(ShaderObject&& other) noexcept;

                GLuint GetHandle() const;
                bool IsValid() const;
            private:
                friend struct ShaderBuilder;
                GLuint mHandle;
            };

            struct ShaderBuilder
            {
                static void Create(const GLenum& type, const std::string& source, ShaderObject& res);
            };

            class ProgramObject
            {
            public:
                ProgramObject();
                ProgramObject(const ProgramObject& other);
                ProgramObject(ProgramObject&& other) noexcept;
                ~ProgramObject();

                ProgramObject& operator=(const ProgramObject& program);
                ProgramObject& operator=(ProgramObject&& program) noexcept;

                GLuint GetHandle() const;
                bool IsValid() const;
            private:
                friend struct ProgramBuilder;
                GLuint mHandle;
            };

            struct ProgramBuilder
            {
                static void Create(ProgramObject& program);
                static void AttachShader(const ShaderObject& shader, const ProgramObject& program);
                static void Link(const ProgramObject& program);
            };
        }
    }
}
#endif // _RENDER_SYSTEM_HPP_
