#include "RenderSystem.hpp"

#include "App.hpp"

App::App(const AppConfig& config, std::shared_ptr<IScene> scene, std::shared_ptr<IUserInterface> ui)
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

App::~App()
{
    mUI->Terminate();
    mWindow->Terminate();
}

void App::Run()
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