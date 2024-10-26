#include <filesystem>
#include <iostream>

#include "RenderSystem.hpp"
#include "Utilities.hpp"

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
        std::cout << "GLSL Version:         " << glslVersion << "\n\n";

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

void Dazzle::RenderSystem::GL::DebugMessageCallback(GLenum source,
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

    assert_with_message(Utilities::kAlwaysFail, message);
}

Dazzle::RenderSystem::GL::VAO::VAO() : mHandle(0)
{
    glCreateVertexArrays(1, &mHandle);
}

Dazzle::RenderSystem::GL::VAO::VAO(const VAO& other) : mHandle(other.mHandle)
{

}

Dazzle::RenderSystem::GL::VAO::VAO(VAO&& other) noexcept : mHandle(other.mHandle) 
{
    other.mHandle = 0;
}

Dazzle::RenderSystem::GL::VAO::~VAO()
{
    if (IsValid())
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

Dazzle::RenderSystem::GL::VBO::VBO() : mHandle(0)
{
    glCreateBuffers(1, &mHandle);
}

Dazzle::RenderSystem::GL::VBO::VBO(const VBO& other) : mHandle(other.mHandle)
{

}

Dazzle::RenderSystem::GL::VBO::VBO(VBO&& other) noexcept : mHandle(other.mHandle) 
{
    other.mHandle = 0;
}

Dazzle::RenderSystem::GL::VBO::~VBO()
{
    if (IsValid())
        glDeleteBuffers(1, &mHandle);
}

Dazzle::RenderSystem::GL::VBO& Dazzle::RenderSystem::GL::VBO::operator=(const VBO& other)
{
    this->mHandle = other.mHandle;
    return *this;
}

Dazzle::RenderSystem::GL::VBO& Dazzle::RenderSystem::GL::VBO::operator=(VBO&& other) noexcept
{
    this->mHandle = other.mHandle;
    other.mHandle = 0;
    return *this;
}

GLuint Dazzle::RenderSystem::GL::VBO::GetHandle() const
{
    return mHandle;
}

bool Dazzle::RenderSystem::GL::VBO::IsValid() const
{
    return mHandle > 0;
}

Dazzle::RenderSystem::GL::EBO::EBO() : mHandle(0)
{
    glCreateBuffers(1, &mHandle);
}

Dazzle::RenderSystem::GL::EBO::EBO(const EBO& other) : mHandle(other.mHandle)
{

}

Dazzle::RenderSystem::GL::EBO::EBO(EBO&& other) noexcept : mHandle(other.mHandle) 
{
    other.mHandle = 0;
}

Dazzle::RenderSystem::GL::EBO::~EBO()
{
    if (IsValid())
        glDeleteBuffers(1, &mHandle);
}

Dazzle::RenderSystem::GL::EBO& Dazzle::RenderSystem::GL::EBO::operator=(const EBO& other)
{
    this->mHandle = other.mHandle;
    return *this;
}

Dazzle::RenderSystem::GL::EBO& Dazzle::RenderSystem::GL::EBO::operator=(EBO&& other) noexcept
{
    this->mHandle = other.mHandle;
    other.mHandle = 0;
    return *this;
}

GLuint Dazzle::RenderSystem::GL::EBO::GetHandle() const
{
    return mHandle;
}

bool Dazzle::RenderSystem::GL::EBO::IsValid() const
{
    return mHandle > 0;
}

Dazzle::RenderSystem::GL::ShaderObject::ShaderObject() : mHandle(0), mSourceCode(std::string()), mType(GLenum())
{

}

Dazzle::RenderSystem::GL::ShaderObject::ShaderObject(const GLenum& type, const std::string& source) : ShaderObject()
{
    ValidateType(type);
    mHandle = glCreateShader(type);
    mSourceCode = source;
}

Dazzle::RenderSystem::GL::ShaderObject::ShaderObject(const ShaderObject& other) : ShaderObject()
{
    mHandle = other.mHandle;
}

Dazzle::RenderSystem::GL::ShaderObject::ShaderObject(ShaderObject&& other) noexcept : ShaderObject()
{
    mHandle = other.mHandle;
    other.mHandle = 0;
}

Dazzle::RenderSystem::GL::ShaderObject::~ShaderObject()
{
    if (IsValid())
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

void Dazzle::RenderSystem::GL::ShaderObject::Compile() const
{
    if (mSourceCode.empty())
    {
        std::cerr << "Unable to Compile Shader: GLSL source code not provided.\n";
        return;
    }

    const GLchar* sourcePtr = mSourceCode.c_str();
    GLint sourceLength = static_cast<GLint>(mSourceCode.size());
    glShaderSource(mHandle, 1, &sourcePtr, &sourceLength);
    glCompileShader(mHandle);
}

void Dazzle::RenderSystem::GL::ShaderObject::Initialize()
{
    if (!IsValid() && mType != 0)
        mHandle = glCreateShader(mType);
}

void Dazzle::RenderSystem::GL::ShaderObject::SetType(const GLenum &type)
{
    ValidateType(type);
    mType = type;
}

void Dazzle::RenderSystem::GL::ShaderObject::SetSourceCode(const std::string &source)
{
    mSourceCode = source;
}

GLint Dazzle::RenderSystem::GL::ShaderObject::GetCompilationStatus() const
{
    GLint compileStatus;
    glGetShaderiv(mHandle, GL_COMPILE_STATUS, &compileStatus);
    return compileStatus;
}

std::string Dazzle::RenderSystem::GL::ShaderObject::GetInfoLog() const
{
    GLint logLength;
    glGetShaderiv(mHandle, GL_INFO_LOG_LENGTH, &logLength);
    std::string log(logLength, '\0');
    glGetShaderInfoLog(mHandle, logLength, nullptr, &log[0]);
    return log;
}

GLuint Dazzle::RenderSystem::GL::ShaderObject::GetHandle() const
{
    return mHandle;
}

bool Dazzle::RenderSystem::GL::ShaderObject::IsValid() const
{
    return mHandle > 0;
}

void Dazzle::RenderSystem::GL::ShaderObject::ValidateType(const GLenum& type)
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
        assert_with_message(Utilities::kAlwaysFail, "Invalid Shader Type.");
    }
}

Dazzle::RenderSystem::GL::ProgramObject::ProgramObject()
{
    mHandle = 0;
    mBinaryFormat = 0;
    mBinary = std::vector<GLubyte>();
}

Dazzle::RenderSystem::GL::ProgramObject::ProgramObject(const ProgramObject& other)
{
    mHandle = other.mHandle;
    mBinaryFormat = other.mBinaryFormat;
    mBinary = other.mBinary;
}

Dazzle::RenderSystem::GL::ProgramObject::ProgramObject(ProgramObject&& other) noexcept
{
    mHandle = other.mHandle;
    mBinaryFormat = other.mBinaryFormat;
    mBinary = std::move(other.mBinary);

    other.mHandle = 0;
    other.mBinaryFormat = 0;
}

Dazzle::RenderSystem::GL::ProgramObject::~ProgramObject()
{
    if (IsValid())
        glDeleteProgram(mHandle);
}

Dazzle::RenderSystem::GL::ProgramObject& Dazzle::RenderSystem::GL::ProgramObject::operator=(const ProgramObject& other)
{
    this->mHandle = other.mHandle;
    this->mBinaryFormat = other.mBinaryFormat;
    this->mBinary = other.mBinary;
    return *this;
}

Dazzle::RenderSystem::GL::ProgramObject& Dazzle::RenderSystem::GL::ProgramObject::operator=(ProgramObject&& other) noexcept
{
    this->mHandle = other.mHandle;
    this->mBinaryFormat = other.mBinaryFormat;
    this->mBinary = std::move(other.mBinary);

    other.mHandle = 0;
    other.mBinaryFormat = 0;
    return *this;
}

void Dazzle::RenderSystem::GL::ProgramObject::Initialize()
{
    mHandle = glCreateProgram();
}

void Dazzle::RenderSystem::GL::ProgramObject::Link() const
{
    if (!IsValid())
    {
        std::cerr << "Unable to Link Program: Invalid Program Object.\n";
        return;
    }

    glLinkProgram(mHandle);
}

void Dazzle::RenderSystem::GL::ProgramObject::AttachShader(const ShaderObject& shader) const
{
    if (!shader.IsValid() || glIsShader(shader.GetHandle()) != GL_TRUE)
    {
        std::cerr << "Unable to attach Shader to Program: Invalid Shader Object.\n";
        return;
    }

    if (!IsValid())
    {
        std::cerr << "Unable to attach Shader to Program: Invalid Program Object.\n";
        return;
    }

    glAttachShader(mHandle, shader.GetHandle());
}

void Dazzle::RenderSystem::GL::ProgramObject::DetachAllShaders() const
{
    auto attachedShaders = GetAttachedShaders();
    for (const auto& shaderHandle : attachedShaders)
    {
        glDetachShader(mHandle, shaderHandle);
    }
}

void Dazzle::RenderSystem::GL::ProgramObject::LoadBinary(const std::vector<char> binary, GLenum format) const
{
    const char* data = binary.data();
    GLsizei length = static_cast<GLsizei>(binary.size());

    glProgramBinary(mHandle, format, data, length);
    if (GetLinkageStatus() != GL_TRUE)
    {
        std::cerr << GetInfoLog();
    }
}

void Dazzle::RenderSystem::GL::ProgramObject::LoadSPIRV() const
{

}

GLint Dazzle::RenderSystem::GL::ProgramObject::GetLinkageStatus() const
{
    GLint linkageStatus;
    glGetProgramiv(mHandle, GL_LINK_STATUS, &linkageStatus);
    return linkageStatus;
}

std::vector<GLuint> Dazzle::RenderSystem::GL::ProgramObject::GetAttachedShaders() const
{
    GLint numShaders = 0;
    glGetProgramiv(mHandle, GL_ATTACHED_SHADERS, &numShaders);
    std::vector<GLuint> attachedShaders(numShaders);
    glGetAttachedShaders(mHandle, numShaders, nullptr, attachedShaders.data());
    return attachedShaders;
}

std::vector<GLubyte> Dazzle::RenderSystem::GL::ProgramObject::GetBinary()
{
    if (mBinary.empty())
        GenerateBinary();

    return mBinary;
}

GLenum Dazzle::RenderSystem::GL::ProgramObject::GetBinaryFormat()
{
    if (mBinaryFormat == 0)
        GenerateBinary();

    return mBinaryFormat;
}

std::string Dazzle::RenderSystem::GL::ProgramObject::GetInfoLog() const
{
    GLint logLength;
    glGetProgramiv(mHandle, GL_INFO_LOG_LENGTH, &logLength);
    std::string log(logLength, '\0');
    glGetProgramInfoLog(mHandle, logLength, nullptr, &log[0]);
    return log;
}

GLuint Dazzle::RenderSystem::GL::ProgramObject::GetHandle() const
{
    return mHandle;
}

bool Dazzle::RenderSystem::GL::ProgramObject::IsValid() const
{
    return mHandle > 0;
}

void Dazzle::RenderSystem::GL::ProgramObject::GenerateBinary()
{
    GLint numFormats = 0;
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &numFormats);
    if (numFormats < 1)
    {
        std::cerr << "Unable to generate Program Binary: Driver does not support binary formats.\n";
        return;
    }

    if (GetLinkageStatus() != GL_TRUE)
    {
        // GL_INVALID_OPERATION
        std::cerr << "Unable to generate Program Binary: Program Object has not been linked.\n";
        return;
    }

    GLint length = 0;
    glGetProgramiv(mHandle, GL_PROGRAM_BINARY_LENGTH, &length);
    mBinary.resize(length);
    glGetProgramBinary(mHandle, length, nullptr, &mBinaryFormat, mBinary.data());
}

void Dazzle::RenderSystem::GL::ShaderBuilder::Build(ShaderObject& shader, const GLenum& type, const std::string& source)
{
    shader.SetType(type);
    shader.SetSourceCode(source);
    shader.Initialize();
    shader.Compile();
    if (shader.GetCompilationStatus() != GL_TRUE)
        std::cerr << shader.GetInfoLog();
}

void Dazzle::RenderSystem::GL::ProgramBuilder::Build(ProgramObject& program, const std::vector<ShaderObject*>& shaders)
{
    for (const auto& shader : shaders)
        /// TODO: Check shader compilation status before attach?
        program.AttachShader(*shader);

    program.Link();
    if (program.GetLinkageStatus() != GL_TRUE)
        std::cerr << program.GetInfoLog();

    program.DetachAllShaders();
    assert_with_message(program.GetAttachedShaders().size() == 0, "OpenGL linked program still contains attached shaders.");
}