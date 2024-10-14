#ifndef _APP_HPP_
#define _APP_HPP_

#include <string>
#include <memory>

#include "Camera.hpp"
#include "Scene.hpp"
#include "UserInterface.hpp"
#include "Window.hpp"

struct AppConfig
{
    int width;
    int height;
    std::string title;
};

class App
{
public:
    App(const AppConfig& config, std::shared_ptr<IScene> scene, std::shared_ptr<IUserInterface> ui);
    ~App();
    void Run();

private:
    std::shared_ptr<Camera> mCamera;
    std::shared_ptr<IScene> mScene;
    std::shared_ptr<IUserInterface> mUI;
    std::shared_ptr<Window> mWindow;
};

#endif // _APP_HPP_