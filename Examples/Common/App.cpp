#include "RenderSystem.hpp"

#include "App.hpp"
#include "Camera.hpp"
#include "Scene.hpp"
#include "UserInterface.hpp"

App::App(const AppConfig& config, std::unique_ptr<IScene> scene, std::unique_ptr<IUserInterface> ui)
{
    // Initialize Window
    mWindow.Initialize(config.width, config.height, config.title, std::move(ui));

    // Create and Initialize Camera
    mCamera = std::make_shared<Camera>();
    mCamera->Initialize(config.width, config.height);

    // Set up OpenGL Debug Message Callback
    Dazzle::RenderSystem::GL::SetupDebugMessageCallback();

    // Initialize Scene
    mScene = std::move(scene);
    mScene->Initialize(mCamera);

    // Add a Scene and Camera references to the UI
    mWindow.GetUserInterface().SetScene(mScene.get());
    mWindow.GetUserInterface().SetCamera(mCamera.get());

    // Register Cursor Input Mode to Keyboard's Callback
    mWindow.GetKeyboard().Register([&cursorMode = this->mCursorMode](int k, int s, int a, int m) { cursorMode.KeyCallback(k, s, a, m); });

    // Register Cursor Input Mode Callbacks
    mCursorMode.Register([&window = this->mWindow](CursorInputMode::Mode mode) { window.CursorInputModeCallback(mode); });
    mCursorMode.Register([camera = mCamera.get()](CursorInputMode::Mode mode) { camera->CursorInputModeCallback(mode); });

    // Register Scene Callbacks
    mWindow.GetFramebuffer().Register([scene = mScene.get()](int w, int h) { scene->FramebufferResizeCallback(w, h); });
    mWindow.GetKeyboard().Register([scene = mScene.get()](int k, int s, int a, int m) { scene->KeyCallback(k, s, a, m); });

    // Register Camera Callbacks
    mWindow.GetCursor().Register([camera = mCamera.get()](double x, double y) { camera->CursorPositionCallback(x, y); });
    mWindow.GetFramebuffer().Register([camera = mCamera.get()](int w, int h) { camera->FramebufferResizeCallback(w, h); });
}

App::~App()
{
    mWindow.Terminate();
}

void App::Run()
{
    while (!mWindow.ShouldClose())
    {
        // Poll GLFW Window Events
        mWindow.PollEvents();

        // Process Input
        mCamera->ProcessInput(mWindow.GetKeyboard());

        // ImGUI New Frame & Update
        mWindow.GetUserInterface().NewFrame();
        mWindow.GetUserInterface().Update();

        // Scene Update & Rendering
        mScene->Update(mWindow.GetTime());
        mScene->Render();

        // ImGUI Rendering
        mWindow.GetUserInterface().Render();

        // GLFW Swap Buffers
        mWindow.SwapBuffers();
    }
}