#ifndef _CAMERA_HPP_
#define _CAMERA_HPP_

#include <iostream>

#include "glm/glm.hpp"

class Keyboard;

class Camera
{
public:
    Camera();

    void Initialize(int width, int height);
    void ProcessInput(const Keyboard& keyboard);

    glm::mat4 GetTransform() const { return mTransform; }
    glm::mat4 GetProjection() const { return mProjection; }
    glm::vec3 GetPosition() const { return mCameraPosition; }
    float GetYaw() const { return mYaw; }
    float GetPitch() const { return mPitch; }

    // Callbacks
    void FramebufferResizeCallback(int width, int height);
    void CursorPositionCallback(double xPosition, double yPosition);
    void CursorInputModeCallback(int mode);
    void KeyCallback(int key, int scancode, int action, int mods);

private:
    unsigned int mBufferWidth;
    unsigned int mBufferHeight;
    float mAspect;
    float mFieldOfView;
    glm::mat4 mTransform;
    glm::mat4 mProjection;
    glm::vec3 mCameraPosition;
    glm::vec3 mFront;
    glm::vec3 mUp;
    float mYaw = 270.0f;
    float mPitch = 0.0f;

    // Camera Movement Settings
    const float kCameraSpeed = 0.05f;
    const float kCameraSensitivity = 0.05f;

    // Cursor position in window coordinates
    glm::vec2 mCursorPosition;

    // Cursor input mode
    int mCursorInputMode;
    bool mCursorInputModeChanged;
};

#endif // _CAMERA_HPP_