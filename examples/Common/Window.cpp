#include <iostream>

#include "Window.hpp"

Window::Window() :  mWindow(nullptr),
                    mCursorInputMode(GLFW_CURSOR_DISABLED),
                    mCursor(),
                    mKeyboard(),
                    mFramebufferObserver()
{
}

void Window::Terminate()
{
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void Window::Initialize(int width, int height, std::string title)
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

    // Disable Cursor by default.
    glfwSetInputMode(mWindow, GLFW_CURSOR, mCursorInputMode);

    // Setup GLFW callbacks
    glfwSetWindowUserPointer(mWindow, this);
    glfwSetCursorPosCallback(mWindow, CursorPosCallback);
    glfwSetKeyCallback(mWindow, KeyCallback);
    glfwSetFramebufferSizeCallback(mWindow, FramebufferResizeCallback);
}

void Window::ErrorCallback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << '\n';
}

void Window::FramebufferResizeCallback(GLFWwindow* GLFWwindow, int width, int height)
{
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(GLFWwindow));

    if (window)
    {
        auto framebufferObserver = window->GetFramebufferObserver();
        framebufferObserver.FramebufferResizeCallback(width, height);
    }
}

void Window::KeyCallback(GLFWwindow* GLFWwindow, int key, int scancode, int action, int mods)
{
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(GLFWwindow));

    if (window)
    {
        // Handle Window Closed
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(GLFWwindow, GLFW_TRUE);
            return;
        }

        // Handle User Cursor Input Mode
        if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
        {
            if (window->GetCursorInputMode() == GLFW_CURSOR_DISABLED)
                window->SetCursorInputMode(GLFW_CURSOR_NORMAL);
            else
                window->SetCursorInputMode(GLFW_CURSOR_DISABLED);

            auto cursor = window->GetCursor();
            cursor.CursorInputModeCallback(window->GetCursorInputMode());
        }

        auto keyboard = window->GetKeyboard();
        keyboard.KeyCallback(key, scancode, action, mods);
    }
}

void Window::CursorPosCallback(GLFWwindow* GLFWwindow, double xPosition, double yPosition)
{
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(GLFWwindow));

    if (window)
    {
        auto cursor = window->GetCursor();
        cursor.CursorPosCallback(xPosition, yPosition);
    }
}