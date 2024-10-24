#ifndef _APP_HPP_
#define _APP_HPP_

#include <string>
#include <memory>

#include "CursorInputMode.hpp"
#include "Window.hpp"

class Camera;
struct IUserInterface;
struct IScene;

struct AppConfig
{
    int width{};
    int height{};
    std::string title;
};

class App
{
public:
    App(const AppConfig& config, std::unique_ptr<IScene> scene, std::unique_ptr<IUserInterface> ui);
    ~App();
    void Run();

private:
    std::shared_ptr<Camera> mCamera;
    std::unique_ptr<IScene> mScene;
    Window mWindow;
    CursorInputMode mCursorMode;
};

#endif // _APP_HPP_