#include "FileManager.hpp"
#include "ShadingEffects.hpp"

Dazzle::ShaderObject::~ShaderObject()
{

}

Dazzle::SimpleShader::SimpleShader() : mProgram(RenderSystem::GL::ProgramId()), mVAO(RenderSystem::GL::VAO())
{
    auto vs = FileLoader::ReadFile("Assets\\Shaders\\SimpleShader.vs.glsl");
    auto fs = FileLoader::ReadFile("Assets\\Shaders\\SimpleShader.fs.glsl");

    RenderSystem::GL::ShaderBuilder shaderBuilder;
    RenderSystem::GL::ProgramBuilder programBuilder;

    RenderSystem::GL::ShaderId vertexShader;
    RenderSystem::GL::ShaderId fragmentShader;
    shaderBuilder.Create(GL_VERTEX_SHADER, vs, vertexShader);
    shaderBuilder.Create(GL_FRAGMENT_SHADER, fs, fragmentShader);

    RenderSystem::GL::ProgramId program;
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
