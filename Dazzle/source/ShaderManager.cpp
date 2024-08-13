#include <iostream>

#include "ShaderManager.hpp"
#include "FileManager.hpp"
#include "Singleton.hpp"

Dazzle::GL::VAO::VAO() : mHandle(0) {}
Dazzle::GL::VAO::VAO(const VAO& other) : mHandle(other.mHandle) {}

Dazzle::GL::VAO::VAO(VAO&& other) noexcept : mHandle(other.mHandle) 
{
    other.mHandle = 0;
}

Dazzle::GL::VAO::~VAO()
{
    if (this->IsValid())
        glDeleteShader(mHandle);
}

Dazzle::GL::VAO& Dazzle::GL::VAO::operator=(const VAO& other)
{
    this->mHandle = other.mHandle;
    return *this;
}

Dazzle::GL::VAO& Dazzle::GL::VAO::operator=(VAO&& other) noexcept
{
    this->mHandle = other.mHandle;
    other.mHandle = 0;
    return *this;
}

GLuint Dazzle::GL::VAO::GetHandle() const
{
    return mHandle;
}

bool Dazzle::GL::VAO::IsValid() const
{
    return mHandle > 0;
}

Dazzle::GL::ShaderId::ShaderId() : mHandle(0) {}
Dazzle::GL::ShaderId::ShaderId(const ShaderId& other) : mHandle(other.mHandle) {}

Dazzle::GL::ShaderId::ShaderId(ShaderId&& other) noexcept : mHandle(other.mHandle) 
{
    other.mHandle = 0;
}

Dazzle::GL::ShaderId::~ShaderId()
{
    if (this->IsValid())
        glDeleteShader(mHandle);
}

Dazzle::GL::ShaderId& Dazzle::GL::ShaderId::operator=(const ShaderId& other)
{
    this->mHandle = other.mHandle;
    return *this;
}

Dazzle::GL::ShaderId& Dazzle::GL::ShaderId::operator=(ShaderId&& other) noexcept
{
    this->mHandle = other.mHandle;
    other.mHandle = 0;
    return *this;
}

GLuint Dazzle::GL::ShaderId::GetHandle() const
{
    return mHandle;
}

bool Dazzle::GL::ShaderId::IsValid() const
{
    return mHandle > 0;
}

Dazzle::GL::ShaderBuilder::ShaderBuilder() {}

Dazzle::GL::ShaderBuilder::~ShaderBuilder() {}

void Dazzle::GL::ShaderBuilder::Create(const GLenum& type, const std::string& source, ShaderId& res)
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


Dazzle::GL::ProgramId::ProgramId() : mHandle(0) {}
Dazzle::GL::ProgramId::ProgramId(const ProgramId& other) : mHandle(other.mHandle) {}

Dazzle::GL::ProgramId::ProgramId(ProgramId&& other) noexcept : mHandle(other.mHandle)
{
    other.mHandle = 0;
}

Dazzle::GL::ProgramId::~ProgramId()
{
    if (this->IsValid())
        glDeleteShader(mHandle);
}

Dazzle::GL::ProgramId& Dazzle::GL::ProgramId::operator=(const ProgramId& other)
{
    this->mHandle = other.mHandle;
    return *this;
}

Dazzle::GL::ProgramId& Dazzle::GL::ProgramId::operator=(ProgramId&& other) noexcept
{
    this->mHandle = other.mHandle;
    other.mHandle = 0;
    return *this;
}

GLuint Dazzle::GL::ProgramId::GetHandle() const
{
    return mHandle;
}

bool Dazzle::GL::ProgramId::IsValid() const
{
    return mHandle > 0;
}

Dazzle::GL::ProgramBuilder::ProgramBuilder() {}

Dazzle::GL::ProgramBuilder::~ProgramBuilder() {}

void Dazzle::GL::ProgramBuilder::Create(ProgramId& program)
{
    program.mHandle = glCreateProgram();
}

void Dazzle::GL::ProgramBuilder::AttachShader(const ShaderId& shader, const ProgramId& program)
{
    if (glIsShader(shader.GetHandle()) != GL_TRUE)
    {
        std::cerr << "Unable to attach Shader to Program: Invalid Shader Object\n";
        return;
    }

    glAttachShader(program.GetHandle(), shader.GetHandle());
}

void Dazzle::GL::ProgramBuilder::Link(const ProgramId& program)
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

Dazzle::ShaderObject::~ShaderObject()
{

}

Dazzle::SimpleShader::SimpleShader() : mProgram(GL::ProgramId()), mVAO(GL::VAO())
{
    auto vs = FileLoader::ReadFile("Assets\\Shaders\\SimpleShader.vs.glsl");
    auto fs = FileLoader::ReadFile("Assets\\Shaders\\SimpleShader.fs.glsl");

    GL::ShaderBuilder shaderBuilder;
    GL::ProgramBuilder programBuilder;

    GL::ShaderId vertexShader;
    GL::ShaderId fragmentShader;
    shaderBuilder.Create(GL_VERTEX_SHADER, vs, vertexShader);
    shaderBuilder.Create(GL_FRAGMENT_SHADER, fs, fragmentShader);

    GL::ProgramId program;
    programBuilder.Create(program);
    programBuilder.AttachShader(vertexShader, program);
    programBuilder.AttachShader(fragmentShader, program);
    programBuilder.Link(program);

    mProgram = std::move(program);
}

Dazzle::SimpleShader::~SimpleShader()
{

}

void Dazzle::SimpleShader::Use() const
{

}


void Dazzle::ShaderManager::UseShader(const ShaderObject* const shader)
{

}


Dazzle::ShaderObject* Dazzle::ShaderManager::GetSimpleShader()
{
    return mSimpleShader.GetInstance();
}
