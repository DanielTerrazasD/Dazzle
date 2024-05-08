#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <stdio.h>

GLuint program, vao;
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
const GLfloat kCameraSpeed = 0.05f;
GLfloat yaw = -90.0f;
GLfloat pitch = 0.0f;
GLfloat lastX = 1280.0f / 2.0f;
GLfloat lastY = 720.0f / 2.0f;
bool firstMouse = true;

GLfloat cube_vertices[] = 
{
    // Front face
    -0.5f, -0.5f, 0.5f, // bottom-left
    0.5f, -0.5f, 0.5f,  // bottom-right
    0.5f, 0.5f, 0.5f,   // top-right
    0.5f, 0.5f, 0.5f,   // top-right
    -0.5f, 0.5f, 0.5f,  // top-left
    -0.5f, -0.5f, 0.5f, // bottom-left
    // Back face
    -0.5f, -0.5f, -0.5f, // bottom-left
    -0.5f, 0.5f, -0.5f,  // top-left
    0.5f, 0.5f, -0.5f,   // top-right
    0.5f, 0.5f, -0.5f,   // top-right
    0.5f, -0.5f, -0.5f,  // bottom-right
    -0.5f, -0.5f, -0.5f, // bottom-left
    // Left face
    -0.5f, 0.5f, 0.5f,   // top-right
    -0.5f, 0.5f, -0.5f,  // top-left
    -0.5f, -0.5f, -0.5f, // bottom-left
    -0.5f, -0.5f, -0.5f, // bottom-left
    -0.5f, -0.5f, 0.5f,  // bottom-right
    -0.5f, 0.5f, 0.5f,   // top-right
    // Right face
    0.5f, 0.5f, 0.5f,   // top-left
    0.5f, -0.5f, 0.5f,  // bottom-left
    0.5f, -0.5f, -0.5f, // bottom-right
    0.5f, -0.5f, -0.5f, // bottom-right
    0.5f, 0.5f, -0.5f,  // top-right
    0.5f, 0.5f, 0.5f,   // top-left
    // Top face
    -0.5f, 0.5f, 0.5f,  // top-right
    0.5f, 0.5f, 0.5f,   // top-left
    0.5f, 0.5f, -0.5f,  // bottom-left
    0.5f, 0.5f, -0.5f,  // bottom-left
    -0.5f, 0.5f, -0.5f, // bottom-right
    -0.5f, 0.5f, 0.5f,  // top-right
    // Bottom face
    -0.5f, -0.5f, 0.5f,  // top-right
    -0.5f, -0.5f, -0.5f, // top-left
    0.5f, -0.5f, -0.5f,  // bottom-left
    0.5f, -0.5f, -0.5f,  // bottom-left
    0.5f, -0.5f, 0.5f,   // bottom-right
    -0.5f, -0.5f, 0.5f   // top-right
};

// Shader sources
const GLchar* vertexSource =
    "#version 460 core\n"
    "layout (location = 0) in vec3 position;\n"
    "out vec3 fragColor;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(position, 1.0f);\n"
    "   fragColor = position;\n"
    "}\n";

const GLchar* fragmentSource =
    "#version 460 core\n"
    "in vec3 fragColor;\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "   color = vec4(fragColor, 1.0f);\n"
    "}\n";


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

GLuint CompileShader(GLenum type, const GLchar* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success = 0;
    GLchar info_log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        std::cout << "Error compiling shader: " << info_log << std::endl;
    }

    return shader;
}

GLuint CreateShaderProgram(const GLchar* vertexShaderSource, const GLchar* fragmentShaderSource)
{
    // Compile shaders
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Create shader program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check for linking errors
    GLint success = 0;
    GLchar info_log[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        std::cout << "Error linking shader program: " << info_log << std::endl;
    }

    // Delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void Init()
{
    // Create Vertex Array Object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create Vertex Buffer Object
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    // Set vertex attribute pointers
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr); // ???
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // Unbind VAO
    glBindVertexArray(0);

    // Create shader program
    program = CreateShaderProgram(vertexSource, fragmentSource);
}

void Display()
{
    glUseProgram(program);
    // Set up transformations
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3( 0.0, 0.0, 0.0));
    glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

    // Get the uniform locations
    GLint modelLoc = glGetUniformLocation(program, "model");
    GLint viewLoc = glGetUniformLocation(program, "view");
    GLint projectionLoc = glGetUniformLocation(program, "projection");

    // Pass the transformation matrices to the shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Draw the cube
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

// Function to handle keyboard input
void ProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPosition += kCameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPosition -= kCameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraPosition += kCameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraPosition -= kCameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * kCameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * kCameraSpeed;
}

// Function to handle keyboard input
void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// Function to handle mouse input
void glfw_mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = float(xpos);
        lastY = float(ypos);
        firstMouse = false; 
    }

    GLdouble xoffset = xpos - lastX;
    GLdouble yoffset = lastY - ypos;
    lastX = float(xpos);
    lastY = float(ypos);

    GLdouble sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += float(xoffset);
    pitch += float(yoffset);

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 460";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW", nullptr, nullptr);
    if (window == nullptr)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup GLFW mouse input
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, glfw_mouse_callback);
    glfwSetKeyCallback(window, glfw_key_callback);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    Init();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ProcessInput(window);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        Display();

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
