#include <string>

#include <GL/gl3w.h>

class Utils
{
public:
    static bool CheckOpenGLError();
    static GLuint CreateShaderProgram(const wchar_t* vertex_shader_path, const wchar_t* fragment_shader_path);

private:
    static void PrintShaderLog(GLuint shader);
    static void PrintProgramLog(GLuint program);
    
    static std::string ReadShaderFile(const wchar_t* file_path);
    static GLuint PrepareShader(int shader_type, const wchar_t* shader_path);
    static GLuint FinalizeShaderProgram(GLuint sprogram);
};