#include <iostream>

#include "RenderSystem.hpp"

void Dazzle::RenderSystem::GL::SetupDebugMessageCallback()
{
    GLint contextFlags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlags);

    if (contextFlags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        std::cout << "Debug Context Created\n";
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(DebugMessageCallback, nullptr);
        // Enable all message's sources, types and severities:
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

        // Filter out specific id:
        GLuint filterOutId = 131185;
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, &filterOutId, GL_FALSE);
    }
}

void Dazzle::RenderSystem::GL::DebugMessageCallback(  GLenum source,
                                        GLenum type,
                                        GLuint id,
                                        GLenum severity,
                                        GLsizei length,
                                        const GLchar* message,
                                        const void* userParam)
{
    std::cout << "----------------------------------------\n";
    std::cout << "|Debug Callback                        |\n";
    std::cout << "----------------------------------------\n";
    std::cout << "ID: " << id << '\n';

    std::cout << "Message Source: ";
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "GL_DEBUG_SOURCE_API\n";             break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "GL_DEBUG_SOURCE_WINDOW_SYSTEM\n";   break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "GL_DEBUG_SOURCE_SHADER_COMPILER\n"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "GL_DEBUG_SOURCE_THIRD_PARTY\n";     break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "GL_DEBUG_SOURCE_APPLICATION\n";     break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "GL_DEBUG_SOURCE_OTHER\n";           break;
    }

    std::cout << "Error Type: ";
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "GL_DEBUG_TYPE_ERROR\n";               break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR\n"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR\n";  break;
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "GL_DEBUG_TYPE_PORTABILITY\n";         break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "GL_DEBUG_TYPE_PERFORMANCE\n";         break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "GL_DEBUG_TYPE_MARKER\n";              break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "GL_DEBUG_TYPE_PUSH_GROUP\n";          break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "GL_DEBUG_TYPE_POP_GROUP\n";           break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "GL_DEBUG_TYPE_OTHER\n";               break;
    }

    std::cout << "Severity: ";
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "GL_DEBUG_SEVERITY_HIGH\n";           break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "GL_DEBUG_SEVERITY_MEDIUM\n";         break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "GL_DEBUG_SEVERITY_LOW\n";            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "GL_DEBUG_SEVERITY_NOTIFICATION\n";   break;
    }

    std::cout << "\nMessage:\n" << message << "\n";
    // Optionally, throw an exception or assert:
    throw std::runtime_error(message);
}