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
    }
}
#endif // _RENDER_SYSTEM_HPP_
