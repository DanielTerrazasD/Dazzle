#include "FileManager.hpp"
#include "ShadingEffects.hpp"

Dazzle::ShadingEffect::~ShadingEffect()
{

}

Dazzle::SimpleShader::SimpleShader() : mProgram(RenderSystem::GL::ProgramObject()), mVAO(RenderSystem::GL::VAO())
{
    auto vssc = FileManager::ReadFile("Assets\\Shaders\\SimpleShader.vs.glsl"); // Vertex Shader Source Code
    auto fssc = FileManager::ReadFile("Assets\\Shaders\\SimpleShader.fs.glsl"); // Fragment Shader Source Code

    RenderSystem::GL::ShaderObject vso; // Vertex Shader Object
    RenderSystem::GL::ShaderObject fso; // Fragment Shader Object

    RenderSystem::GL::ShaderBuilder::Build(vso, GL_VERTEX_SHADER, vssc);
    RenderSystem::GL::ShaderBuilder::Build(fso, GL_FRAGMENT_SHADER, fssc);

    RenderSystem::GL::ProgramObject program;
    RenderSystem::GL::ProgramBuilder::Build(program, {&vso, &fso});

    // Save the program binary
    // auto binary = program.GetBinary();
    // auto format = program.GetBinaryFormat();
    // char* data = reinterpret_cast<char*>(binary.data());
    // size_t length = binary.size();
    // FileManager::WriteBinary("Assets\\Shaders\\SimpleShader.bin", data, length);

    mProgram = std::move(program);
}

Dazzle::SimpleShader::~SimpleShader()
{

}

void Dazzle::SimpleShader::Use() const
{

}
