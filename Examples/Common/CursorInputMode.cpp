#include "GLFW/glfw3.h"

#include "CursorInputMode.hpp"

CursorInputMode::Mode CursorInputMode::GetInputMode() const
{
    return mMode;
}

void CursorInputMode::Register(std::function<void(Mode)> callback)
{
    mObservers.push_back(callback);
}

void CursorInputMode::KeyCallback(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
    {
        if (mMode == Mode::Disabled)
            mMode = Mode::Normal;
        else
            mMode = Mode::Disabled;

        Notify(mMode);
    }
}

void CursorInputMode::Notify(Mode mode)
{
    for (const auto& observerCallback : mObservers)
        observerCallback(mode);
}