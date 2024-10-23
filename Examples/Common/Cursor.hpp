#ifndef _CURSOR_HPP_
#define _CURSOR_HPP_

#include <functional>
#include <vector>

class Window;
struct GLFWwindow;

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

#endif // _CURSOR_HPP_