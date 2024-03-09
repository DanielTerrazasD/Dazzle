#include "Utils.h"

#include <iostream>
#include <fstream>
#include <sstream>

bool Utils::CheckOpenGLError()
{
    bool found_error = false;
    int gl_error = glGetError();
    while (gl_error != GL_NO_ERROR)
    {
        std::cout << "glError: " << gl_error << '\n';
        found_error = true;
        gl_error = glGetError();
    }

    return found_error;
}

GLuint Utils::CreateShaderProgram(const wchar_t* vertex_shader_path, const wchar_t* fragment_shader_path)
{
    GLuint vertex_shader = PrepareShader(GL_VERTEX_SHADER, vertex_shader_path);
    GLuint fragment_shader = PrepareShader(GL_FRAGMENT_SHADER, fragment_shader_path);
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    FinalizeShaderProgram(program);
    return program;
}

void Utils::PrintShaderLog(GLuint shader)
{
    GLint info_log_length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0)
    {
        std::string info_log(info_log_length, '\0');
        glGetShaderInfoLog(shader, info_log_length, nullptr, &info_log[0]);
        std::cerr << "Compilation error:\n" << info_log << '\n';
    }
    throw;
}

void Utils::PrintProgramLog(GLuint program)
{
    GLint info_log_length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0)
    {
        std::string info_log(info_log_length, '\0');
        glGetProgramInfoLog(program, info_log_length, nullptr, &info_log[0]);
        std::cerr << "Program error:\n" << info_log << '\n';
    }
    throw;
}

std::string Utils::ReadShaderFile(const wchar_t* file_path)
{
    std::fstream file(file_path, std::ios::in);

    if (!file.is_open())
    {
        std::cerr << "<ERROR>: Could not open file:\n" << file_path << '\n';
        throw;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint Utils::PrepareShader(int shader_type, const wchar_t* shader_path)
{
    std::string data = ReadShaderFile(shader_path);
    const char* shader_source = data.c_str();
    GLint shader = glCreateShader(shader_type);

    if (shader == 0 || shader == GL_INVALID_ENUM)
    {
        std::cerr   << "<ERROR>: Could not create shader" << shader_path
                    << " of type; " << shader_type << '\n';
        throw;
    }

    glShaderSource(shader, 1, &shader_source, nullptr);
    glCompileShader(shader);
    CheckOpenGLError();

    GLint shader_compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_compiled);
    if (shader_compiled != GL_TRUE)
    {
        std::cout << "<ERROR> ";
        if (shader_type == GL_VERTEX_SHADER) 
            std::cout << "[VERTEX_SHADER] ";
        if (shader_type == GL_TESS_CONTROL_SHADER) 
            std::cout << "[TESS_CONTROL_SHADER] ";
        if (shader_type == GL_TESS_EVALUATION_SHADER) 
            std::cout << "[TESS_EVALUATION_SHADER] ";
        if (shader_type == GL_GEOMETRY_SHADER) 
            std::cout << "[GEOMETRY_SHADER] ";
        if (shader_type == GL_FRAGMENT_SHADER) 
            std::cout << "[FRAGMENT_SHADER] ";
        if (shader_type == GL_COMPUTE_SHADER) 
            std::cout << "[COMPUTE_SHADER] ";
        PrintShaderLog(shader);
    }

    return shader;
}

GLuint Utils::FinalizeShaderProgram(GLuint sprogram)
{
    glLinkProgram(sprogram);
    CheckOpenGLError();

    GLint program_linked;
    glGetProgramiv(sprogram, GL_LINK_STATUS, &program_linked);
    if (program_linked != GL_TRUE)
    {
        std::cerr << "<ERROR> Program Linking Failed\n";
        PrintProgramLog(sprogram);
    }
    return sprogram;
}