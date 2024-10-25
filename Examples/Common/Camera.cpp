#include <algorithm>
#include <mutex>

#include "glm/gtc/matrix_transform.hpp"

#include "Camera.hpp"
#include "Keyboard.hpp"

constexpr float DegreesToRadians(float degrees)
{
    return (float)((degrees * 3.1415926) / 180.0);
}

Camera::Camera() :  mBufferWidth(0),
                    mBufferHeight(0),
                    mFieldOfView(0),
                    mAspect(0),
                    mProjection(glm::mat4()),
                    mCursorPosition(glm::vec2()),
                    mCameraPosition(glm::vec3()),
                    mFront(glm::vec3()),
                    mUp(glm::vec3()),
                    mTransform(glm::mat4()),
                    mCursorInputMode(CursorInputMode::Mode::Disabled),
                    mCursorInputModeChanged(true)
{
}

void Camera::Initialize(int width, int height)
{
    mBufferWidth = width;
    mBufferHeight = height;
    mFieldOfView = 60.0f;
    mAspect = static_cast<float>(mBufferWidth) / static_cast<float>(mBufferHeight);
    mProjection = glm::perspective(DegreesToRadians(mFieldOfView), mAspect, 0.1f, 1000.0f);

    mCursorPosition = glm::vec2(0.0f, 0.0f);
    mCameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);

    mFront = glm::vec3(0.0f, 0.0f, -1.0f);
    mUp = glm::vec3(0.0f, 1.0f, 0.0f);
    mTransform = glm::lookAt(mCameraPosition, mCameraPosition + mFront, mUp);
}

void Camera::ProcessInput(const Keyboard& keyboard)
{
    if (keyboard.IsKeyPressed(Keyboard::Key::W))
        mCameraPosition += kCameraSpeed * mFront;
    if (keyboard.IsKeyPressed(Keyboard::Key::S))
        mCameraPosition -= kCameraSpeed * mFront;
    if (keyboard.IsKeyPressed(Keyboard::Key::E))
        mCameraPosition += kCameraSpeed * mUp;
    if (keyboard.IsKeyPressed(Keyboard::Key::Q))
        mCameraPosition -= kCameraSpeed * mUp;
    if (keyboard.IsKeyPressed(Keyboard::Key::A))
        mCameraPosition -= glm::normalize(glm::cross(mFront, mUp)) * kCameraSpeed;
    if (keyboard.IsKeyPressed(Keyboard::Key::D))
        mCameraPosition += glm::normalize(glm::cross(mFront, mUp)) * kCameraSpeed;

    mTransform = glm::lookAt(mCameraPosition, mCameraPosition + mFront, mUp);
}

void Camera::FramebufferResizeCallback(int width, int height)
{
    mBufferWidth = width;
    mBufferHeight = height;
    mAspect = static_cast<float>(mBufferWidth) / static_cast<float>(mBufferHeight);
    mProjection = glm::perspective(DegreesToRadians(mFieldOfView), mAspect, 0.1f, 1000.0f);
}

void Camera::CursorPositionCallback(double xPosition, double yPosition)
{
    if (mCursorInputMode == CursorInputMode::Mode::Normal)
        return;

    if (mCursorInputModeChanged)
    {
        mCursorPosition.x = float(xPosition);
        mCursorPosition.y = float(yPosition);
        mCursorInputModeChanged = false;
    }

    double xOffset = xPosition - mCursorPosition.x;
    double yOffset = mCursorPosition.y - yPosition;
    mCursorPosition.x = float(xPosition);
    mCursorPosition.y = float(yPosition);

    xOffset *= kCameraSensitivity;
    yOffset *= kCameraSensitivity;

    mYaw += float(xOffset);
    mPitch += float(yOffset);

    mYaw = std::fmod(mYaw, 360.0f);
    mPitch = std::clamp(mPitch, -89.0f, 89.0f);

    glm::vec3 front;
    front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    front.y = sin(glm::radians(mPitch));
    front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
    mFront = glm::normalize(front);

    mTransform = glm::lookAt(mCameraPosition, mCameraPosition + mFront, mUp);
}

void Camera::CursorInputModeCallback(CursorInputMode::Mode mode)
{
    if (mCursorInputMode != mode)
    {
        mCursorInputModeChanged = true;
        mCursorInputMode = mode;
    }
}