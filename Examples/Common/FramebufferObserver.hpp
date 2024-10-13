#ifndef _FRAMEBUFFER_OBSERVER_HPP_
#define _FRAMEBUFFER_OBSERVER_HPP_

#include <functional>
#include <vector>

class FramebufferObserver
{
public:
    void FramebufferResizeCallback(int width, int height)
    {
        Notify(width, height);
    }

    void Register(std::function<void(int,int)> callback)
    {
        mObservers.push_back(callback);
    }

private:
    void Notify(int width, int height)
    {
        for (const auto& observerCallback : mObservers)
            observerCallback(width, height);
    }

    std::vector<std::function<void(int,int)>> mObservers;
};

#endif // _FRAMEBUFFER_OBSERVER_HPP_