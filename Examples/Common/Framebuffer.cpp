#include "pch.hpp"

#include "Framebuffer.hpp"
#include "Window.hpp"

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