#ifndef _FRAMEBUFFER_HPP_
#define _FRAMEBUFFER_HPP_

#include <functional>
#include <vector>

class Window;
struct GLFWwindow;

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

#endif // _FRAMEBUFFER_HPP_