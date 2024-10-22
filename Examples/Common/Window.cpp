#include "pch.hpp"

#include <iostream>

#include "UserInterface.hpp"
#include "Window.hpp"

void Cursor::Register(std::function<void(double,double)> callback)
{
    mObservers.push_back(callback);
}

void Cursor::Notify(double xPosition, double yPosition)
{
    for (const auto& observerCallback : mObservers)
        observerCallback(xPosition, yPosition);
}

void Cursor::CursorPosCallback(GLFWwindow* GLFWwindow, double xPosition, double yPosition)
{
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(GLFWwindow));
    if (window)
        window->GetCursor().Notify(xPosition, yPosition);
}

void Framebuffer::Register(std::function<void(int,int)> callback)
{
    mObservers.push_back(callback);
}

void Framebuffer::Notify(int width, int height)
{
    for (const auto& observerCallback : mObservers)
        observerCallback(width, height);
}

void Framebuffer::FramebufferResizeCallback(GLFWwindow* GLFWwindow, int width, int height)
{
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(GLFWwindow));
    if (window)
        window->GetFramebuffer().Notify(width, height);
}

Keyboard::Keyboard(GLFWwindow* window) : mWindow(window), mObservers()
{
    mKeyMap = {
        {Key::W, false},
        {Key::A, false},
        {Key::S, false},
        {Key::D, false},
        {Key::Q, false},
        {Key::E, false},
        {Key::ESC, false},
        {Key::TAB, false},
    };
}

void Keyboard::Register(std::function<void(int,int,int,int)> callback)
{
    mObservers.push_back(callback);
}

void Keyboard::Notify(int key, int scancode, int action, int mods)
{
    for (const auto& observerCallback : mObservers)
        observerCallback(key, scancode, action, mods);
}

void Keyboard::PollKeys()
{
    mKeyMap[Key::W] = glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS ? true : false;
    mKeyMap[Key::A] = glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS ? true : false;
    mKeyMap[Key::S] = glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS ? true : false;
    mKeyMap[Key::D] = glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS ? true : false;
    mKeyMap[Key::Q] = glfwGetKey(mWindow, GLFW_KEY_Q) == GLFW_PRESS ? true : false;
    mKeyMap[Key::E] = glfwGetKey(mWindow, GLFW_KEY_E) == GLFW_PRESS ? true : false;
    mKeyMap[Key::ESC] = glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS ? true : false;
    mKeyMap[Key::TAB] = glfwGetKey(mWindow, GLFW_KEY_TAB) == GLFW_PRESS ? true : false;
}

void Keyboard::KeyCallback(GLFWwindow* GLFWwindow, int key, int scancode, int action, int mods)
{
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(GLFWwindow));
    if (window)
        window->GetKeyboard().Notify(key, scancode, action, mods);
}

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