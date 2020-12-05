#pragma once

#include "raylib.h"
#include "raymath.h"
#include <functional>

class FPCamera
{
public:

    FPCamera();

    void Setup(float scale, float fovY, Vector3&& position);
    void ViewResized();

    void Update();

    float GetFOVX() const;
    Vector2 GetMapPosition() const;
    Vector3 GetCameraPosition() const;

    inline Vector2 GetViewAngles() const { return Vector2Scale(Angle, 1.0f / DEG2RAD); }

   typedef enum
    {
        MOVE_FRONT = 0,
        MOVE_BACK,
        MOVE_RIGHT,
        MOVE_LEFT,
        MOVE_UP,
        MOVE_DOWN,
        TURN_LEFT,
        TURN_RIGHT,
        TURN_UP,
        TURN_DOWN,
        SPRINT,
        LAST_CONTROL
    }CameraControls;

    int ControlsKeys[LAST_CONTROL];

    Vector3 MoveSpeed = { 1, 1, 1 };
    Vector2 TurnSpeed = { 90, 90 };

    float MouseSensitivity = 600;

    float MinimumViewY = -65.0f;
    float MaximumViewY = 89.0f;

    float ViewBobbleFreq = 0.0f;
    float ViewBobbleMagnatude = 0.02f;
    float ViewBobbleWaverMagnitude = 0.002f;

    typedef std::function<bool(FPCamera& camera, Vector3& newPostion, const Vector3& oldPostion)> PostionCallback;
    PostionCallback ValidateCamPostion = nullptr;
    PostionCallback ValidateMapPostion = nullptr;

    inline const Camera& GetCamera() const { return ViewCamera; }

    bool UseMouseX = true;
    bool UseMouseY = true;

protected:
    float ViewScale = 1;
    bool Focused = true;
    Vector3 CameraPosition;

    Camera ViewCamera;
    Vector2 FOV;

    Vector2 PreviousMousePosition = { 0.0f, 0.0f };

    float TargetDistance = 0;               // Camera distance from position to target
    float PlayerEyesPosition = 0.5f;       // Player eyes position from ground (in meters)
    Vector2 Angle = { 0,0 };                // Camera angle in plane XZ

    float CurrentBobble = 0;

    float GetSpeedForAxis(CameraControls axis, float speed);
};