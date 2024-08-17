#include <cassert>
#include <iostream>

#include "RenderSystem.hpp"

void Dazzle::RenderSystem::GL::SetupDebugMessageCallback()
{
    GLint contextFlags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlags);

    if (contextFlags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        const GLubyte* renderer = glGetString(GL_RENDERER);
        const GLubyte* vendor = glGetString(GL_VENDOR);
        const GLubyte* glVersion = glGetString(GL_VERSION);
        const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

        GLint major;
        GLint minor;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);

        std::cout << "Debug Context Created\n";
        std::cout << "GL Renderer:          " << renderer << '\n';
        std::cout << "GL Vendor:            " << vendor << '\n';
        std::cout << "GL Version (string):  " << glVersion << '\n';
        std::cout << "GL Version (integer): " << major << '.' << minor << '\n';
        std::cout << "GLSL Version:         " << glslVersion << '\n';

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


Dazzle::RenderSystem::GL::VAO::VAO() : mHandle(0) {}
Dazzle::RenderSystem::GL::VAO::VAO(const VAO& other) : mHandle(other.mHandle) {}

Dazzle::RenderSystem::GL::VAO::VAO(VAO&& other) noexcept : mHandle(other.mHandle) 
{
    other.mHandle = 0;
}

Dazzle::RenderSystem::GL::VAO::~VAO()
{
    if (this->IsValid())
        glDeleteVertexArrays(1, &mHandle);
}

Dazzle::RenderSystem::GL::VAO& Dazzle::RenderSystem::GL::VAO::operator=(const VAO& other)
{
    this->mHandle = other.mHandle;
    return *this;
}

Dazzle::RenderSystem::GL::VAO& Dazzle::RenderSystem::GL::VAO::operator=(VAO&& other) noexcept
{
    this->mHandle = other.mHandle;
    other.mHandle = 0;
    return *this;
}

GLuint Dazzle::RenderSystem::GL::VAO::GetHandle() const
{
    return mHandle;
}

bool Dazzle::RenderSystem::GL::VAO::IsValid() const
{
    return mHandle > 0;
}

Dazzle::RenderSystem::GL::ShaderObject::ShaderObject() : mHandle(0) {}
Dazzle::RenderSystem::GL::ShaderObject::ShaderObject(const ShaderObject& other) : mHandle(other.mHandle) {}

Dazzle::RenderSystem::GL::ShaderObject::ShaderObject(ShaderObject&& other) noexcept : mHandle(other.mHandle) 
{
    other.mHandle = 0;
}

Dazzle::RenderSystem::GL::ShaderObject::~ShaderObject()
{
    if (this->IsValid())
        glDeleteShader(mHandle);
}

Dazzle::RenderSystem::GL::ShaderObject& Dazzle::RenderSystem::GL::ShaderObject::operator=(const ShaderObject& other)
{
    this->mHandle = other.mHandle;
    return *this;
}

Dazzle::RenderSystem::GL::ShaderObject& Dazzle::RenderSystem::GL::ShaderObject::operator=(ShaderObject&& other) noexcept
{
    this->mHandle = other.mHandle;
    other.mHandle = 0;
    return *this;
}

GLuint Dazzle::RenderSystem::GL::ShaderObject::GetHandle() const
{
    return mHandle;
}

bool Dazzle::RenderSystem::GL::ShaderObject::IsValid() const
{
    return mHandle > 0;
}

void Dazzle::RenderSystem::GL::ShaderBuilder::Create(const GLenum& type, const std::string& source, ShaderObject& res)
{
    switch (type)
    {
    case GL_VERTEX_SHADER:
    case GL_TESS_CONTROL_SHADER:
    case GL_TESS_EVALUATION_SHADER:
    case GL_GEOMETRY_SHADER:
    case GL_FRAGMENT_SHADER:
    case GL_COMPUTE_SHADER:
        break;

    default:
        // Error: GL_INVALID_ENUM
        res.mHandle = 0;
        return;
    }

    res.mHandle = glCreateShader(type);
    const GLchar* sourcePtr = source.c_str();
    GLint sourceLength = static_cast<GLint>(source.size());
    glShaderSource(res.mHandle, 1, &sourcePtr, &sourceLength);
    glCompileShader(res.mHandle);

    GLint compileStatus;
    glGetShaderiv(res.mHandle, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus != GL_TRUE)
    {
        GLint logLength;
        glGetShaderiv(res.mHandle, GL_INFO_LOG_LENGTH, &logLength);
        std::string log(logLength, '\0');
        glGetShaderInfoLog(res.mHandle, logLength, &logLength, &log[0]);
        std::cerr << "Shader compilation failed:\n" << log << '\n';
    }
}


Dazzle::RenderSystem::GL::ProgramObject::ProgramObject() : mHandle(0) {}
Dazzle::RenderSystem::GL::ProgramObject::ProgramObject(const ProgramObject& other) : mHandle(other.mHandle) {}

Dazzle::RenderSystem::GL::ProgramObject::ProgramObject(ProgramObject&& other) noexcept : mHandle(other.mHandle)
{
    other.mHandle = 0;
}

Dazzle::RenderSystem::GL::ProgramObject::~ProgramObject()
{
    if (this->IsValid())
        glDeleteProgram(mHandle);
}

Dazzle::RenderSystem::GL::ProgramObject& Dazzle::RenderSystem::GL::ProgramObject::operator=(const ProgramObject& other)
{
    this->mHandle = other.mHandle;
    return *this;
}

Dazzle::RenderSystem::GL::ProgramObject& Dazzle::RenderSystem::GL::ProgramObject::operator=(ProgramObject&& other) noexcept
{
    this->mHandle = other.mHandle;
    other.mHandle = 0;
    return *this;
}

GLuint Dazzle::RenderSystem::GL::ProgramObject::GetHandle() const
{
    return mHandle;
}

bool Dazzle::RenderSystem::GL::ProgramObject::IsValid() const
{
    return mHandle > 0;
}

void Dazzle::RenderSystem::GL::ProgramBuilder::Create(ProgramObject& program)
{
    program.mHandle = glCreateProgram();
}

void Dazzle::RenderSystem::GL::ProgramBuilder::AttachShader(const ShaderObject& shader, const ProgramObject& program)
{
    if (glIsShader(shader.GetHandle()) != GL_TRUE)
    {
        std::cerr << "Unable to attach Shader to Program: Invalid Shader Object\n";
        return;
    }

    glAttachShader(program.GetHandle(), shader.GetHandle());
}

void Dazzle::RenderSystem::GL::ProgramBuilder::Link(const ProgramObject& program)
{
    glLinkProgram(program.GetHandle());

    GLint linkageStatus;
    glGetProgramiv(program.GetHandle(), GL_LINK_STATUS, &linkageStatus);
    if (linkageStatus != GL_TRUE)
    {
        GLint logLength;
        glGetProgramiv(program.GetHandle(), GL_INFO_LOG_LENGTH, &logLength);
        std::string log(logLength, '\0');
        glGetProgramInfoLog(program.GetHandle(), logLength, &logLength, &log[0]);
        std::cerr << "Program linkage failed:\n" << log << '\n';
    }

    GLint nAttachedShaders;     // Number of attached shaders.
    GLint cAttachedShaders;     // Counter of retrieved attached shaders.
    glGetProgramiv(program.GetHandle(), GL_ATTACHED_SHADERS, &nAttachedShaders);
    std::vector<GLuint> attachedShaders(nAttachedShaders);
    glGetAttachedShaders(program.GetHandle(), nAttachedShaders, &cAttachedShaders, attachedShaders.data());

    // Counters of attached shaders must be equal.
    assert(nAttachedShaders == cAttachedShaders);

    // Detach all attached shaders to this already linked program.
    for (const auto shaderHandle : attachedShaders)
    {
        if (shaderHandle > 0)
            glDetachShader(program.GetHandle(), shaderHandle);
    }
}