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


Dazzle::RenderSystem::GL::VAO::VAO() : mHandle(0) {}
Dazzle::RenderSystem::GL::VAO::VAO(const VAO& other) : mHandle(other.mHandle) {}

Dazzle::RenderSystem::GL::VAO::VAO(VAO&& other) noexcept : mHandle(other.mHandle) 
{
    other.mHandle = 0;
}

Dazzle::RenderSystem::GL::VAO::~VAO()
{
    if (this->IsValid())
        glDeleteShader(mHandle);
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

Dazzle::RenderSystem::GL::ShaderId::ShaderId() : mHandle(0) {}
Dazzle::RenderSystem::GL::ShaderId::ShaderId(const ShaderId& other) : mHandle(other.mHandle) {}

Dazzle::RenderSystem::GL::ShaderId::ShaderId(ShaderId&& other) noexcept : mHandle(other.mHandle) 
{
    other.mHandle = 0;
}

Dazzle::RenderSystem::GL::ShaderId::~ShaderId()
{
    if (this->IsValid())
        glDeleteShader(mHandle);
}

Dazzle::RenderSystem::GL::ShaderId& Dazzle::RenderSystem::GL::ShaderId::operator=(const ShaderId& other)
{
    this->mHandle = other.mHandle;
    return *this;
}

Dazzle::RenderSystem::GL::ShaderId& Dazzle::RenderSystem::GL::ShaderId::operator=(ShaderId&& other) noexcept
{
    this->mHandle = other.mHandle;
    other.mHandle = 0;
    return *this;
}

GLuint Dazzle::RenderSystem::GL::ShaderId::GetHandle() const
{
    return mHandle;
}

bool Dazzle::RenderSystem::GL::ShaderId::IsValid() const
{
    return mHandle > 0;
}

Dazzle::RenderSystem::GL::ShaderBuilder::ShaderBuilder() {}

Dazzle::RenderSystem::GL::ShaderBuilder::~ShaderBuilder() {}

void Dazzle::RenderSystem::GL::ShaderBuilder::Create(const GLenum& type, const std::string& source, ShaderId& res)
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


Dazzle::RenderSystem::GL::ProgramId::ProgramId() : mHandle(0) {}
Dazzle::RenderSystem::GL::ProgramId::ProgramId(const ProgramId& other) : mHandle(other.mHandle) {}

Dazzle::RenderSystem::GL::ProgramId::ProgramId(ProgramId&& other) noexcept : mHandle(other.mHandle)
{
    other.mHandle = 0;
}

Dazzle::RenderSystem::GL::ProgramId::~ProgramId()
{
    if (this->IsValid())
        glDeleteShader(mHandle);
}

Dazzle::RenderSystem::GL::ProgramId& Dazzle::RenderSystem::GL::ProgramId::operator=(const ProgramId& other)
{
    this->mHandle = other.mHandle;
    return *this;
}

Dazzle::RenderSystem::GL::ProgramId& Dazzle::RenderSystem::GL::ProgramId::operator=(ProgramId&& other) noexcept
{
    this->mHandle = other.mHandle;
    other.mHandle = 0;
    return *this;
}

GLuint Dazzle::RenderSystem::GL::ProgramId::GetHandle() const
{
    return mHandle;
}

bool Dazzle::RenderSystem::GL::ProgramId::IsValid() const
{
    return mHandle > 0;
}

Dazzle::RenderSystem::GL::ProgramBuilder::ProgramBuilder() {}

Dazzle::RenderSystem::GL::ProgramBuilder::~ProgramBuilder() {}

void Dazzle::RenderSystem::GL::ProgramBuilder::Create(ProgramId& program)
{
    program.mHandle = glCreateProgram();
}

void Dazzle::RenderSystem::GL::ProgramBuilder::AttachShader(const ShaderId& shader, const ProgramId& program)
{
    if (glIsShader(shader.GetHandle()) != GL_TRUE)
    {
        std::cerr << "Unable to attach Shader to Program: Invalid Shader Object\n";
        return;
    }

    glAttachShader(program.GetHandle(), shader.GetHandle());
}

void Dazzle::RenderSystem::GL::ProgramBuilder::Link(const ProgramId& program)
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
}