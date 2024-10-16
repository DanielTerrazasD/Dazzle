#include <algorithm>
#include <mutex>

#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "Camera.hpp"
#include "Utilities.hpp"

Camera::Camera() :  mBufferWidth(0),
                    mBufferHeight(0),
                    mFieldOfView(0),
                    mAspect(0),
                    mProjection(glm::mat4()),
                    mCursorPosition(glm::vec2()),
                    mCameraPosition(glm::vec3()),
                    mFront(glm::vec3()),
                    mUp(glm::vec3()),
                    mTransform(glm::mat4())
{
}

void Camera::Initialize(int width, int height, int cursorInputMode)
{
    mBufferWidth = width;
    mBufferHeight = height;
    mFieldOfView = 60.0f;
    mAspect = static_cast<float>(mBufferWidth) / static_cast<float>(mBufferHeight);
    mProjection = glm::perspective(Dazzle::Utilities::DegreesToRadians(mFieldOfView), mAspect, 0.1f, 1000.0f);

    mCursorPosition = glm::vec2(0.0f, 0.0f);
    mCameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    mCursorInputMode = cursorInputMode;
    mCursorInputModeChanged = true;

    mFront = glm::vec3(0.0f, 0.0f, -1.0f);
    mUp = glm::vec3(0.0f, 1.0f, 0.0f);
    mTransform = glm::lookAt(mCameraPosition, mCameraPosition + mFront, mUp);
}

void Camera::ProcessInput(GLFWwindow* const window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        mCameraPosition += kCameraSpeed * mFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        mCameraPosition -= kCameraSpeed * mFront;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        mCameraPosition += kCameraSpeed * mUp;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        mCameraPosition -= kCameraSpeed * mUp;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        mCameraPosition -= glm::normalize(glm::cross(mFront, mUp)) * kCameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        mCameraPosition += glm::normalize(glm::cross(mFront, mUp)) * kCameraSpeed;

    mTransform = glm::lookAt(mCameraPosition, mCameraPosition + mFront, mUp);
}

void Camera::FramebufferResizeCallback(int width, int height)
{
    mBufferWidth = width;
    mBufferHeight = height;
    mAspect = static_cast<float>(mBufferWidth) / static_cast<float>(mBufferHeight);
    mProjection = glm::perspective(Dazzle::Utilities::DegreesToRadians(mFieldOfView), mAspect, 0.1f, 1000.0f);
}

void Camera::CursorPositionCallback(double xPosition, double yPosition)
{
    if (mCursorInputMode == GLFW_CURSOR_NORMAL)
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

void Camera::CursorInputModeCallback(int mode)
{
    if (mCursorInputMode != mode)
    {
        mCursorInputModeChanged = true;
        mCursorInputMode = mode;
    }
}

void Camera::KeyCallback(int key, int scancode, int action, int mods)
{

}
