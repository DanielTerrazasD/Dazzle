#ifndef _WINDOW_HPP_
#define _WINDOW_HPP_

#include <memory>

#include "GLFW/glfw3.h"

#include "CursorObserver.hpp"
#include "KeyboardObserver.hpp"
#include "FramebufferObserver.hpp"

class Window
{
public:
    Window();
    void Initialize(int width, int height, std::string title);
    void Terminate();
    GLFWwindow* GetHandle() const { return mWindow; }

    static void ErrorCallback(int error, const char* description);
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void CursorPosCallback(GLFWwindow* window, double xPosition, double yPosition);

    CursorObserver& GetCursorObserver() { return mCursorObserver; }
    KeyboardObserver& GetKeyboardObserver() { return mKeyboardObserver; }
    FramebufferObserver& GetFramebufferObserver() { return mFramebufferObserver; }

    void SetCursorInputMode(int mode) { mCursorInputMode = mode; glfwSetInputMode(mWindow, GLFW_CURSOR, mCursorInputMode); }
    int GetCursorInputMode() const { return mCursorInputMode; }

    double GetTime() const { return glfwGetTime(); }
    bool ShouldClose() const { return glfwWindowShouldClose(mWindow); }
    void PollEvents() { glfwPollEvents(); }
    void SwapBuffers() { glfwSwapBuffers(mWindow); }

private:
    GLFWwindow* mWindow;
    int mCursorInputMode;

    CursorObserver mCursorObserver;
    KeyboardObserver mKeyboardObserver;
    FramebufferObserver mFramebufferObserver;
};

#endif // _WINDOW_HPP_
