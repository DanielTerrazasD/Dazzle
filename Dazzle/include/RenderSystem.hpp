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
            void SetupDebugMessageCallback(const std::vector<GLuint>& filterOutIDs = {131185});
            void DebugMessageCallback(  GLenum source,
                                        GLenum type,
                                        GLuint id,
                                        GLenum severity,
                                        GLsizei length,
                                        const GLchar *message,
                                        const void *userParam);

            // Vertex Array Object
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

            // Vertex Buffer Object
            class VBO
            {
            public:
                VBO();
                VBO(const VBO& other);
                VBO(VBO&& other) noexcept;
                ~VBO();

                VBO& operator=(const VBO& program);
                VBO& operator=(VBO&& program) noexcept;

                GLuint GetHandle() const;
                bool IsValid() const;
            private:
                GLuint mHandle;
            };

            // Element Buffer Object
            class EBO
            {
            public:
                EBO();
                EBO(const EBO& other);
                EBO(EBO&& other) noexcept;
                ~EBO();

                EBO& operator=(const EBO& program);
                EBO& operator=(EBO&& program) noexcept;

                GLuint GetHandle() const;
                bool IsValid() const;
            private:
                GLuint mHandle;
            };

            class ShaderObject
            {
            public:
                ShaderObject();
                ShaderObject(const GLenum& type, const std::string& source);
                ShaderObject(const ShaderObject& other);
                ShaderObject(ShaderObject&& other) noexcept;
                ~ShaderObject();

                ShaderObject& operator=(const ShaderObject& other);
                ShaderObject& operator=(ShaderObject&& other) noexcept;

                void Compile() const;
                void Initialize();

                void SetType(const GLenum& type);
                void SetSourceCode(const std::string& source);
                GLint GetCompilationStatus() const;
                std::string GetInfoLog() const;
                GLuint GetHandle() const;
                std::string GetShaderTypeAsString();
                bool IsValid() const;
            private:
                void ValidateType(const GLenum& type);

                GLuint mHandle;
                GLenum mType;
                std::string mSourceCode;
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

                void Initialize();
                void Link() const;
                void AttachShader(const ShaderObject& shader) const;
                void DetachAllShaders() const;
                void LoadBinary(const std::vector<char> binary, GLenum format) const;
                void LoadSPIRV() const;

                GLint GetLinkageStatus() const;
                std::vector<GLuint> GetAttachedShaders() const;
                std::vector<GLubyte> GetBinary();
                GLenum GetBinaryFormat();
                std::string GetInfoLog() const;
                GLuint GetHandle() const;
                bool IsValid() const;
            private:
                void GenerateBinary();

                GLuint mHandle;
                std::vector<GLubyte> mBinary;
                GLenum mBinaryFormat;
            };

            namespace ShaderBuilder
            {
                void Build(ShaderObject& shader, const GLenum& type, const std::string& source);
            };

            namespace ProgramBuilder
            {
                void Build(ProgramObject& program, const std::vector<ShaderObject*>& shaders);
            };
        }
    }
}
#endif // _RENDER_SYSTEM_HPP_
