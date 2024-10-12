#ifndef _APP_HPP_
#define _APP_HPP_

#include <string>

#include "RenderSystem.hpp"

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
    App(const AppConfig& config, std::shared_ptr<IScene> scene, std::shared_ptr<IUserInterface> ui)
    {
        // Create and Initialize Window
        mWindow = std::make_shared<Window>();
        mWindow->Initialize(config.width, config.height, config.title);

        // Initialize User Interface
        mUI = ui;
        mUI->Initialize(mWindow);

        // Create and Initialize Camera
        mCamera = std::make_shared<Camera>();
        mCamera->Initialize(config.width, config.height, mWindow->GetCursorInputMode());

        // Set up OpenGL Debug Message Callback
        Dazzle::RenderSystem::GL::SetupDebugMessageCallback();

        // Initialize Scene
        mScene = scene;
        mScene->Initialize(mCamera);

        // Add a Scene and Camera references to the UI
        mUI->SetScene(mScene);
        mUI->SetCamera(mCamera);

        // Register Scene Callbacks
        mWindow->GetFramebufferObserver().Register([scene = mScene](int w, int h) { scene->FramebufferResizeCallback(w, h); });
        mWindow->GetKeyboardObserver().Register([scene = mScene](int k, int s, int a, int m) { scene->KeyCallback(k, s, a, m); });

        // Register Camera Callbacks
        mWindow->GetCursorObserver().Register([this](double x, double y) { mCamera->CursorPositionCallback(x, y); });
        mWindow->GetCursorObserver().Register([this](int m) { mCamera->CursorInputModeCallback(m); });
        mWindow->GetFramebufferObserver().Register([this](int w, int h) { mCamera->FramebufferResizeCallback(w, h); });
    }

    ~App()
    {
        mUI->Terminate();
        mWindow->Terminate();
    }

    void Run()
    {
        while (!mWindow->ShouldClose())
        {
            mWindow->PollEvents();
            mCamera->ProcessInput(mWindow->GetHandle());
            mUI->NewFrame();
            mUI->Update();
            mScene->Update(mWindow->GetTime());
            mScene->Render();
            mUI->Render();
            mWindow->SwapBuffers();
        }
    }

private:
    std::shared_ptr<Camera> mCamera;
    std::shared_ptr<IScene> mScene;
    std::shared_ptr<IUserInterface> mUI;
    std::shared_ptr<Window> mWindow;
};

#endif // _APP_HPP_