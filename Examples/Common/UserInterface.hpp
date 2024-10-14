#ifndef _USER_INTERFACE_HPP_
#define _USER_INTERFACE_HPP_

#include "Scene.hpp"
#include "Window.hpp"

struct IUserInterface
{
    virtual ~IUserInterface() {}

    virtual void Initialize(const std::shared_ptr<Window>& window);
    virtual void Terminate();
    virtual void NewFrame();
    virtual void Render();

    virtual void Update() = 0;
    virtual void SetScene(const std::shared_ptr<IScene>&) = 0;
    virtual void SetCamera(const std::shared_ptr<Camera>&) = 0;

};

#endif // _USER_INTERFACE_HPP_