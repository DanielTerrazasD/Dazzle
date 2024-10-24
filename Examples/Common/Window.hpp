#ifndef _WINDOW_HPP_
#define _WINDOW_HPP_

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "Cursor.hpp"
#include "CursorInputMode.hpp"
#include "Framebuffer.hpp"
#include "Keyboard.hpp"

struct GLFWwindow;
struct IUserInterface;

class Window
{
public:
    Window();

    void Initialize(int width, int height, std::string title, std::unique_ptr<IUserInterface> ui);
    void Terminate();

    bool ShouldClose() const;
    void PollEvents() const;
    void SwapBuffers() const;

    Cursor& GetCursor() { return *mCursor; }
    Keyboard& GetKeyboard() { return *mKeyboard; }
    Framebuffer& GetFramebuffer() { return *mFramebuffer; }
    IUserInterface& GetUserInterface() { return *mUserInterface; }
    double GetTime() const;

    // Callbacks
    void CursorInputModeCallback(CursorInputMode::Mode mode);

private:
    GLFWwindow* mWindow;
    int mCursorInputMode;

    std::unique_ptr<Cursor> mCursor;
    std::unique_ptr<Framebuffer> mFramebuffer;
    std::unique_ptr<Keyboard> mKeyboard;
    std::unique_ptr<IUserInterface> mUserInterface;

    // Callbacks
    void KeyCallback(int key, int scancode, int action, int mods);
    static void ErrorCallback(int error, const char* description);
};

#endif // _WINDOW_HPP_
