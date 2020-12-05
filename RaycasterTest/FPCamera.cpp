#include <cmath>
#include "FPCamera.h"
#include "raymath.h"

FPCamera::FPCamera() : ControlsKeys{ 'W', 'S', 'D', 'A', 'E', 'Q', -1, -1, -1, -1, KEY_LEFT_SHIFT }
{

}

void FPCamera::Setup(float scale, float fovY, Vector3&& position)
{
    CameraPosition = position;
    ViewCamera.position = { scale * position.x, position.y * scale, position.z * scale };
    ViewCamera.position.y += PlayerEyesPosition;
    ViewCamera.target = Vector3Add(ViewCamera.position, Vector3{ 0, 0, 1 });
    ViewCamera.up = { 0.0f, 1.0f, 0.0f };
    ViewCamera.fovy = fovY;
    ViewCamera.type = CAMERA_PERSPECTIVE;

    Focused = IsWindowFocused();
    if (Focused)
        DisableCursor();

    TargetDistance = 1;

    ViewResized();
    PreviousMousePosition = GetMousePosition();
}

void FPCamera::ViewResized()
{
    float width = (float)GetScreenWidth();
    float height = (float)GetScreenHeight();

    FOV.y = ViewCamera.fovy;

    if (height != 0)
        FOV.x = FOV.y * (width / height);
}

float FPCamera::GetSpeedForAxis(CameraControls axis, float speed)
{
    int key = ControlsKeys[axis];
    if (key == -1)
        return 0;

    float factor = 1.0f;
    if (IsKeyDown(ControlsKeys[SPRINT]))
        factor = 2;

    if (IsKeyDown(ControlsKeys[axis]))
        return speed * GetFrameTime() * factor;

    return 0.0f;
}

void FPCamera::Update()
{
    if (IsWindowFocused() != Focused)
    {
        Focused = IsWindowFocused();
        if (Focused)
        {
            DisableCursor();
            PreviousMousePosition = GetMousePosition(); // so there is no jump on focus
        }
        else
        {
            EnableCursor();
        }
    }

    // Mouse movement detection
    Vector2 mousePositionDelta = { 0.0f, 0.0f };
    Vector2 mousePosition = GetMousePosition();
    float mouseWheelMove = GetMouseWheelMove();

    // Keys input detection
    float direction[MOVE_DOWN+1] = {  GetSpeedForAxis(MOVE_FRONT,MoveSpeed.z),
                                      GetSpeedForAxis(MOVE_BACK,MoveSpeed.z),
                                      GetSpeedForAxis(MOVE_RIGHT,MoveSpeed.x),
                                      GetSpeedForAxis(MOVE_LEFT,MoveSpeed.x),
                                      GetSpeedForAxis(MOVE_UP,MoveSpeed.y),
                                      GetSpeedForAxis(MOVE_DOWN,MoveSpeed.y) };

    mousePositionDelta.x = mousePosition.x - PreviousMousePosition.x;
    mousePositionDelta.y = mousePosition.y - PreviousMousePosition.y;

    PreviousMousePosition = mousePosition;

    Vector3 forward = Vector3Subtract(ViewCamera.target,ViewCamera.position);
    forward.y = 0;
    forward = Vector3Normalize(forward);

    Vector3 right{ forward.z * -1.0f, 0, forward.x };

    Vector3 oldPostion = CameraPosition;

    CameraPosition = Vector3Add(CameraPosition, Vector3Scale(forward, direction[MOVE_FRONT] - direction[MOVE_BACK]));
    CameraPosition = Vector3Add(CameraPosition, Vector3Scale(right, direction[MOVE_RIGHT] - direction[MOVE_LEFT]));

    CameraPosition.y += direction[MOVE_UP] - direction[MOVE_DOWN];

    // let someone modify the projected position
    if (ValidateMapPostion != nullptr)
    {
        Vector3 camMapPos = Vector3Scale(CameraPosition, 1.0f / ViewScale);
        Vector3 prevMapPos = Vector3Scale(oldPostion, 1.0f / ViewScale);

        if (!ValidateMapPostion(*this, camMapPos, prevMapPos))
            CameraPosition = Vector3Scale(camMapPos, ViewScale);
    }
    
    if (ValidateCamPostion != nullptr)
        ValidateCamPostion(*this, CameraPosition, oldPostion);

    // Camera orientation calculation
    float turnRotation = GetSpeedForAxis(TURN_RIGHT, TurnSpeed.x) - GetSpeedForAxis(TURN_LEFT, TurnSpeed.x);
    float tiltRotation = GetSpeedForAxis(TURN_UP, TurnSpeed.y) - GetSpeedForAxis(TURN_DOWN, TurnSpeed.y);

    if (turnRotation != 0)
        Angle.x -= turnRotation * DEG2RAD;
    else if (UseMouseX && Focused)
        Angle.x += (mousePositionDelta.x / -MouseSensitivity);
    
    if (tiltRotation)
        Angle.y += tiltRotation * DEG2RAD;
    else if (UseMouseY && Focused)
        Angle.y += (mousePositionDelta.y / -MouseSensitivity);

        // Angle clamp
    if (Angle.y < MinimumViewY * DEG2RAD)
        Angle.y = MinimumViewY * DEG2RAD;
    else if (Angle.y > MaximumViewY * DEG2RAD) 
       Angle.y = MaximumViewY * DEG2RAD;

        // Recalculate camera target considering translation and rotation
    Matrix translation = MatrixTranslate(0, 0, (TargetDistance / 5.1f));
    Matrix rotation = MatrixRotateXYZ(Vector3{ PI * 2 - Angle.y, PI * 2 - Angle.x, 0 });
    Matrix transform = MatrixMultiply(translation, rotation);

    ViewCamera.position = CameraPosition;

    float eyeOfset = PlayerEyesPosition;

    if (ViewBobbleFreq > 0)
    {
        float swingDelta = std::fmax(std::abs(direction[MOVE_FRONT] - direction[MOVE_BACK]), std::abs(direction[MOVE_RIGHT] - direction[MOVE_LEFT]));

        // If movement detected (some key pressed), increase swinging
        CurrentBobble += swingDelta * ViewBobbleFreq;

        constexpr float ViewBobbleDampen = 8.0f;

        eyeOfset -= sinf(CurrentBobble / ViewBobbleDampen) * ViewBobbleMagnatude;

        ViewCamera.up.x = sinf(CurrentBobble / (ViewBobbleDampen * 2)) * ViewBobbleWaverMagnitude;
        ViewCamera.up.z = -sinf(CurrentBobble / (ViewBobbleDampen * 2)) * ViewBobbleWaverMagnitude;
    }
    else
    {
        CurrentBobble = 0;
        ViewCamera.up.x = 0;
        ViewCamera.up.z = 0;
    }

    ViewCamera.position.y += eyeOfset;

    ViewCamera.target.x = ViewCamera.position.x - transform.m12;
    ViewCamera.target.y = ViewCamera.position.y - transform.m13;
    ViewCamera.target.z = ViewCamera.position.z - transform.m14;
}

float FPCamera::GetFOVX() const
{
    return FOV.x;
}

Vector2 FPCamera::GetMapPosition() const
{
    return Vector2{ std::floor(CameraPosition.x / ViewScale), std::floor(CameraPosition.z / ViewScale) };
}

Vector3 FPCamera::GetCameraPosition() const
{
    return CameraPosition;
}
