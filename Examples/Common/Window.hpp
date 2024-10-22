#ifndef _WINDOW_HPP_
#define _WINDOW_HPP_

#include <functional>
#include <memory>
#include <unordered_map>

struct GLFWwindow;
struct IUserInterface;

class Cursor
{
public:
    Cursor() : mObservers() {}
    void Register(std::function<void(double,double)> callback);

private:
    void Notify(double xPosition, double yPosition);
    static void CursorPosCallback(GLFWwindow* window, double xPosition, double yPosition);
    std::vector<std::function<void(double,double)>> mObservers;
    friend class Window;
};

class Framebuffer
{
public:
    Framebuffer() : mObservers() {}
    void Register(std::function<void(int,int)> callback);

private:
    void Notify(int width, int height);
    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    std::vector<std::function<void(int,int)>> mObservers;
    friend class Window;
};

class Keyboard
{
public:
    enum class Key
    {
        W,
        A,
        S,
        D,
        Q,
        E,
        ESC,
        TAB
    };

    Keyboard(GLFWwindow* window);
    void Register(std::function<void(int,int,int,int)> callback);
    std::unordered_map<Key, bool> GetKeyMap() const { return mKeyMap; }
    bool IsKeyPressed(Key key) const { return mKeyMap.at(key); }

private:
    void Notify(int key, int scancode, int action, int mods);
    void PollKeys();

    // Callbacks
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    GLFWwindow* mWindow;
    std::unordered_map<Key, bool> mKeyMap;
    std::vector<std::function<void(int,int,int,int)>> mObservers;
    friend class Window;
};

class Window
{
public:
    Window();

    void Initialize(int width, int height, std::string title,  std::unique_ptr<IUserInterface> ui);
    void Terminate();

    bool ShouldClose() const;
    void PollEvents() const;
    void SwapBuffers() const;

    Cursor& GetCursor() { return *mCursor; }
    Keyboard& GetKeyboard() { return *mKeyboard; }
    Framebuffer& GetFramebuffer() { return *mFramebuffer; }
    IUserInterface& GetUserInterface() { return *mUserInterface; }
    double GetTime() const;

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
