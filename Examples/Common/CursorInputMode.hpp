#ifndef _CURSOR_INPUT_MODE_HPP_
#define _CURSOR_INPUT_MODE_HPP_

#include <functional>

class CursorInputMode
{
public:
    enum class Mode
    {
        Normal,
        Hidden,
        Disabled
    };

    CursorInputMode() : mMode(Mode::Disabled) {}
    Mode GetInputMode() const;

    void Register(std::function<void(Mode)> callback);
    void KeyCallback(int key, int scancode, int action, int mods);

private:
    void Notify(Mode mode);

    Mode mMode;
    std::vector<std::function<void(Mode)>> mObservers;
};

#endif // _CURSOR_INPUT_MODE_HPP_