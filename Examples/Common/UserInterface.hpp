#ifndef _USER_INTERFACE_HPP_
#define _USER_INTERFACE_HPP_

#include <memory>

struct GLFWwindow;
struct IScene;
class Camera;

struct IUserInterface
{
    virtual ~IUserInterface() {}

    virtual void Initialize(GLFWwindow* window);
    virtual void Terminate();
    virtual void NewFrame();
    virtual void Render();

    virtual void Update() = 0;
    virtual void SetScene(const std::shared_ptr<IScene>&) = 0;
    virtual void SetCamera(const std::shared_ptr<Camera>&) = 0;
};

#endif // _USER_INTERFACE_HPP_