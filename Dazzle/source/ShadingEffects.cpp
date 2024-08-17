#include "FileManager.hpp"
#include "ShadingEffects.hpp"

Dazzle::ShadingEffect::~ShadingEffect()
{

}

Dazzle::SimpleShader::SimpleShader() : mProgram(RenderSystem::GL::ProgramObject()), mVAO(RenderSystem::GL::VAO())
{
    auto vs = FileLoader::ReadFile("Assets\\Shaders\\SimpleShader.vs.glsl");
    auto fs = FileLoader::ReadFile("Assets\\Shaders\\SimpleShader.fs.glsl");

    RenderSystem::GL::ShaderBuilder shaderBuilder;
    RenderSystem::GL::ProgramBuilder programBuilder;

    RenderSystem::GL::ShaderObject vertexShader;
    RenderSystem::GL::ShaderObject fragmentShader;
    shaderBuilder.Create(GL_VERTEX_SHADER, vs, vertexShader);
    shaderBuilder.Create(GL_FRAGMENT_SHADER, fs, fragmentShader);

    RenderSystem::GL::ProgramObject program;
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
