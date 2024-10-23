#ifndef _KEYBOARD_HPP_
#define _KEYBOARD_HPP_

#include <functional>
#include <vector>

class Window;
struct GLFWwindow;

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

#endif // _KEYBOARD_HPP_