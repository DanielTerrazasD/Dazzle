#include "RenderSystem.hpp"

#include "App.hpp"

App::App(const AppConfig& config, std::shared_ptr<IScene> scene, std::unique_ptr<IUserInterface> ui)
{
    // Create and Initialize Window
    mWindow = std::make_shared<Window>();
    mWindow->Initialize(config.width, config.height, config.title, std::move(ui));

    // Create and Initialize Camera
    mCamera = std::make_shared<Camera>();
    mCamera->Initialize(config.width, config.height);

    // Set up OpenGL Debug Message Callback
    Dazzle::RenderSystem::GL::SetupDebugMessageCallback();

    // Initialize Scene
    mScene = scene;
    mScene->Initialize(mCamera);

    // Add a Scene and Camera references to the UI
    mWindow->GetUserInterface().SetScene(mScene);
    mWindow->GetUserInterface().SetCamera(mCamera);

    // Register Scene Callbacks
    mWindow->GetFramebuffer().Register([scene = mScene](int w, int h) { scene->FramebufferResizeCallback(w, h); });
    mWindow->GetKeyboard().Register([scene = mScene](int k, int s, int a, int m) { scene->KeyCallback(k, s, a, m); });

    // Register Camera Callbacks
    mWindow->GetCursor().Register([this](double x, double y) { mCamera->CursorPositionCallback(x, y); });
    // mWindow->GetCursor().Register([this](int m) { mCamera->CursorInputModeCallback(m); });
    mWindow->GetFramebuffer().Register([this](int w, int h) { mCamera->FramebufferResizeCallback(w, h); });
}

App::~App()
{
    mWindow->Terminate();
}

void App::Run()
{
    while (!mWindow->ShouldClose())
    {
        // Poll GLFW Window Events
        mWindow->PollEvents();

        // Process Input
        mCamera->ProcessInput(mWindow->GetKeyboard());

        // ImGUI New Frame & Update
        mWindow->GetUserInterface().NewFrame();
        mWindow->GetUserInterface().Update();

        // Scene Update & Rendering
        mScene->Update(mWindow->GetTime());
        mScene->Render();

        // ImGUI Rendering
        mWindow->GetUserInterface().Render();

        // GLFW Swap Buffers
        mWindow->SwapBuffers();
    }
}