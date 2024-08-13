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

        }
    }
}
#endif // _RENDER_SYSTEM_HPP_
