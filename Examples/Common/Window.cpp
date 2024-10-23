#include "pch.hpp"

#include <iostream>

#include "UserInterface.hpp"
#include "Window.hpp"

Window::Window() :  mWindow(nullptr),
                    mCursor(nullptr),
                    mKeyboard(nullptr),
                    mFramebuffer(nullptr),
                    mUserInterface(nullptr)
{
}

void Window::Initialize(int width, int height, std::string title, std::unique_ptr<IUserInterface> ui)
{
    // GLFW Configuration:
    glfwSetErrorCallback(ErrorCallback);
    if (!glfwInit())
    {
        std::cerr << "GLFW initialization failed.\n";
        return;
    }

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    // Create window with graphics context
    mWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (mWindow == nullptr)
    {
        std::cerr << "GLFW window initialization failed.\n";
        return;
    }

    glfwMakeContextCurrent(mWindow);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GL3W OpenGL loader
    if (gl3wInit() != GL3W_OK)
    {
        std::cerr << "Failed to initialize OpenGL loader!\n";
        return;
    }

    // Disable Cursor by default.
    glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Setup GLFW callbacks
    glfwSetWindowUserPointer(mWindow, this);
    glfwSetCursorPosCallback(mWindow, Cursor::CursorPosCallback);
    glfwSetFramebufferSizeCallback(mWindow, Framebuffer::FramebufferResizeCallback);
    glfwSetKeyCallback(mWindow, Keyboard::KeyCallback);

    // Initialize Cursor
    mCursor = std::make_unique<Cursor>();
    // Initialize Framebuffer
    mFramebuffer = std::make_unique<Framebuffer>();
    // Initialize Keyboard
    mKeyboard = std::make_unique<Keyboard>(mWindow);

    // Register Window KeyCallback
    mKeyboard->Register([this](int k, int s, int a, int m) { KeyCallback(k, s, a, m); });


    mUserInterface = std::move(ui);
    mUserInterface->Initialize(mWindow);
}

void Window::Terminate()
{
    mUserInterface->Terminate();

    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

bool Window::ShouldClose() const
{
    return glfwWindowShouldClose(mWindow);
}

void Window::PollEvents() const
{
    glfwPollEvents();
    mKeyboard->PollKeys();
}

void Window::SwapBuffers() const
{
    glfwSwapBuffers(mWindow);
}

double Window::GetTime() const
{
    return glfwGetTime();
}

void Window::KeyCallback(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(mWindow, GLFW_TRUE);
}

void Window::ErrorCallback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << '\n';
}