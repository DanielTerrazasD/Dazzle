#ifndef _WINDOW_HPP_
#define _WINDOW_HPP_

#include <memory>

#include "CursorObserver.hpp"
#include "KeyboardObserver.hpp"
#include "FramebufferObserver.hpp"

struct GLFWwindow;
class Window
{
public:
    Window();

    void Initialize(int width, int height, std::string title);
    void Terminate();

    bool ShouldClose() const;
    void PollEvents() const;
    void SwapBuffers() const;

    void SetCursorInputMode(int mode);

    CursorObserver& GetCursorObserver() { return mCursorObserver; }
    KeyboardObserver& GetKeyboardObserver() { return mKeyboardObserver; }
    FramebufferObserver& GetFramebufferObserver() { return mFramebufferObserver; }

    GLFWwindow* GetHandle() const;
    int GetCursorInputMode() const;
    double GetTime() const;

    // Callbacks
    static void ErrorCallback(int error, const char* description);
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void CursorPosCallback(GLFWwindow* window, double xPosition, double yPosition);


private:
    GLFWwindow* mWindow;
    int mCursorInputMode;

    CursorObserver mCursorObserver;
    KeyboardObserver mKeyboardObserver;
    FramebufferObserver mFramebufferObserver;
};

#endif // _WINDOW_HPP_
