#include "FileManager.hpp"
#include "ShadingEffects.hpp"

Dazzle::ShadingEffect::~ShadingEffect()
{

}

Dazzle::SimpleShader::SimpleShader() : mProgram(RenderSystem::GL::ProgramObject()), mVAO(RenderSystem::GL::VAO())
{
    auto vssc = FileManager::ReadFileFrom("Assets\\Shaders\\SimpleShader.vs.glsl"); // Vertex Shader Source Code
    auto fssc = FileManager::ReadFileFrom("Assets\\Shaders\\SimpleShader.fs.glsl"); // Fragment Shader Source Code

    RenderSystem::GL::ShaderObject vso; // Vertex Shader Object
    RenderSystem::GL::ShaderObject fso; // Fragment Shader Object

    RenderSystem::GL::ShaderBuilder::Build(vso, GL_VERTEX_SHADER, vssc);
    RenderSystem::GL::ShaderBuilder::Build(fso, GL_FRAGMENT_SHADER, fssc);

    RenderSystem::GL::ProgramObject program;
    RenderSystem::GL::ProgramBuilder::Build(program, {&vso, &fso});

    mProgram = std::move(program);
}

Dazzle::SimpleShader::~SimpleShader()
{

}

void Dazzle::SimpleShader::Use() const
{

}
