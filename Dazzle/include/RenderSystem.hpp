#ifndef _RENDER_SYSTEM_HPP_
#define _RENDER_SYSTEM_HPP_

#include <string>
#include <vector>

#include <GL/gl3w.h>

#include "DazzleEngine.hpp"

namespace DazzleEngine
{
    namespace RenderSystem
    {
        namespace GLSL
        {
        }

        namespace GL
        {
            class Shader
            {
            public:
                Shader();
                GLuint& CreateShader(const GLenum type, const std::string& source);
                ~Shader();
            
            private:
                GLuint mShader;
            };

            class Program
            {
                Program();
                void AttachShader(const GLuint shader);
                GLuint GetProgramHandle();
                ~Program();

            private:
                GLuint mProgram;
            };
            
            // class ProgramManager {}; // Keeps track of all the created programs. Delete them when the application finishes

            void SetupDebugMessageCallback();
            void DebugOutput(   GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar *message,
                                const void *userParam);

            // class ShaderManager {};
        }
    }
}
#endif // _RENDER_SYSTEM_HPP_
