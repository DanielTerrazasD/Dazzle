#ifndef _CURSOR_HPP_
#define _CURSOR_HPP_

#include <functional>
#include <vector>

class CursorObserver
{
public:
    void CursorPositionCallback(double xPosition, double yPosition)
    {
        Notify(xPosition, yPosition);
    }

    void CursorInputModeCallback(int mode)
    {
        Notify(mode);
    }

    void Register(std::function<void(double,double)> callback)
    {
        mCursorPositionObservers.push_back(callback);
    }

    void Register(std::function<void(int)> callback)
    {
        mCursorInputModeObservers.push_back(callback);
    }

private:
    void Notify(double xPosition, double yPosition)
    {
        for (const auto& observerCallback : mCursorPositionObservers)
            observerCallback(xPosition, yPosition);
    }

    void Notify(int mode)
    {
        for (const auto& observerCallback : mCursorInputModeObservers)
            observerCallback(mode);
    }

    std::vector<std::function<void(double,double)>> mCursorPositionObservers;
    std::vector<std::function<void(int)>> mCursorInputModeObservers;
};

#endif // _CURSOR_HPP_