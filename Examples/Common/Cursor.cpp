#include "pch.hpp"

#include "Cursor.hpp"
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