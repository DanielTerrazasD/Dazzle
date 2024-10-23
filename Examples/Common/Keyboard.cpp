#include "pch.hpp"

#include "Keyboard.hpp"
#include "Window.hpp"

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