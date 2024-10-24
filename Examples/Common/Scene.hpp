#ifndef _SCENE_HPP_
#define _SCENE_HPP_

#include <string>

#include "Camera.hpp"

struct IScene
{
public:
    virtual ~IScene() {}
    virtual void Initialize(const std::shared_ptr<Camera>& camera) = 0;
    virtual void Render() = 0;
    virtual void Update(double time) = 0;

    // Callbacks
    virtual void FramebufferResizeCallback(int width, int height) = 0;
    virtual void KeyCallback(int key, int scancode, int action, int mods) = 0;
    virtual void CursorCallback(double xPosition, double yPosition) = 0;

protected:

};

#endif // _SCENE_HPP_