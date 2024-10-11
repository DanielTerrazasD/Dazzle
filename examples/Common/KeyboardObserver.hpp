#ifndef _KEYBOARD_HPP_
#define _KEYBOARD_HPP_

#include <functional>
#include <vector>

class KeyboardObserver
{
public:
    void KeyCallback(int key, int scancode, int action, int mods)
    {
        Notify(key, scancode, action, mods);
    }

    void Register(std::function<void(int,int,int,int)> callback)
    {
        mObservers.push_back(callback);
    }

private:
    void Notify(int key, int scancode, int action, int mods)
    {
        for (const auto& observerCallback : mObservers)
            observerCallback(key, scancode, action, mods);
    }

    std::vector<std::function<void(int,int,int,int)>> mObservers;
};

#endif // _KEYBOARD_HPP_