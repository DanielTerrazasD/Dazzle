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

GLuint program, VAO, VBO;
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
    "uniform mat4 model_view;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * model_view * vec4(position, 1.0f);\n"
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


inline float DegreesToRadians(float degrees)
{
    return (degrees * 3.14159265359f) / 180.0f;
}

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
// -------------------------------------------------------------------------------------------------
    // Create Vertex Array Object
    glCreateVertexArrays(1, &VAO);
// -------------------------------------------------------------------------------------------------
    // Create Vertex Buffer Object
    glCreateBuffers(1, &VBO);

    // Initialize buffer storage (data store)
    glNamedBufferStorage(VBO, sizeof(cube_vertices), cube_vertices, 0);
// -------------------------------------------------------------------------------------------------
    const GLintptr kOffset = 0;
    const GLintptr kStride = 0;
    const GLuint kBindingIndex = 0; // Binding Index (from 0 to GL_MAX_VERTEX_ATTRIB_BINDINGS)
    const GLuint kAttribIndex = 0;  // Vertex Attribute Index
    const GLuint kSize = 3;         // The number of values per vertex that are stored in the array.
    const GLenum kDataType = GL_FLOAT;
    const GLboolean kNormalized = GL_FALSE;

    // Bind buffer to vertex array
    glVertexArrayVertexBuffer(  VAO,                        // Vertex Array Object
                                kBindingIndex,              // Binding Index
                                VBO,                        // Vertex Buffer Object
                                kOffset,                    // Offset (Offset of the first element)
                                3 * sizeof(float));         // Stride (Distance between elements within the buffer)

    // Specify the format for the given attribute
    glVertexArrayAttribFormat(  VAO,                        // Vertex Array Object
                                kAttribIndex,               // (Vertex) Attribute Index
                                kSize,                      // Size (Number of values per vertex that are stored in the array)
                                kDataType,                  // Data Type
                                kNormalized,                // Normalized (if parameter represents a normalized integer)
                                kOffset);                   // Relative Offset

    // Specify which vertex buffer binding to use for this attribute
    glVertexArrayAttribBinding( VAO,                        // Vertex Array Object
                                kAttribIndex,               // (Vertex) Attribute Index
                                kBindingIndex);             // Binding Index

    // Enable the attribute
    glEnableVertexArrayAttrib(  VAO,                        // Vertex Array Object
                                kAttribIndex);              // (Vertex) Attribute Index
// -------------------------------------------------------------------------------------------------

    // Create shader program
    program = CreateShaderProgram(vertexSource, fragmentSource);
}

void Display(double current_time)
{
    glUseProgram(program);
    // Set up transformations
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));
    glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

    glm::mat4 model_view = view * model;

    // Get the uniform locations
    GLint modelViewLoc = glGetUniformLocation(program, "model_view");
    // GLint modelLoc = glGetUniformLocation(program, "model");
    // GLint viewLoc = glGetUniformLocation(program, "view");
    GLint projectionLoc = glGetUniformLocation(program, "projection");

    // Pass the transformation matrices to the shader
    glUniformMatrix4fv(modelViewLoc, 1, GL_FALSE, glm::value_ptr(model_view));
    // glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    // glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Draw the cube
    glBindVertexArray(VAO);

    glEnable(GL_CULL_FACE);     // GL_CULL_FACE: Cull polygons based on their winding order.
    glCullFace(GL_BACK);        // glCullFace(): Specify whether front- or back-facing are culled.
    glFrontFace(GL_CCW);        // GL_CCW//GL_CW: Specify the orientation of front-facing polygons.

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

        Display(glfwGetTime());

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
