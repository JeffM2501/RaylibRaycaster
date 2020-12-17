/**********************************************************************************************
*
*   FPCamera - A first person camera system for use by raylib (raylib.com)
*
*
*   LICENSE: zlib/libpng
*
*   FPCamera is licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software:
*
*   Copyright (c) 2020 Jeffery Myers, based on camera code in raylib
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "FPCamera.h"
#include <math.h>

void InitFPCamera(FPCamera* camera, float fovY, Vector3 position)
{
	if (camera == NULL)
		return;

	camera->ControlsKeys[0] = 'W'; 
	camera->ControlsKeys[1] = 'S';
	camera->ControlsKeys[2] = 'D';
	camera->ControlsKeys[3] = 'A';
	camera->ControlsKeys[4] = 'E';
	camera->ControlsKeys[5] = 'Q';
	camera->ControlsKeys[6] = KEY_LEFT;
	camera->ControlsKeys[7] = KEY_RIGHT;
	camera->ControlsKeys[8] = KEY_UP;
	camera->ControlsKeys[9] = KEY_DOWN;
	camera->ControlsKeys[10] = KEY_LEFT_SHIFT;


	camera->MoveSpeed = { 1,1,1 };
	camera->TurnSpeed = { 90,90 };

	camera->UseMouse = true;
	camera->MouseSensitivity = 600;

	camera->MinimumViewY = -89.0f;
	camera->MaximumViewY = 89.0f;

	camera->ViewBobbleFreq = 0.0f;
	camera->ViewBobbleMagnatude = 0.02f;
	camera->ViewBobbleWaverMagnitude = 0.002f;
	camera->CurrentBobble = 0;

	camera->PreviousMousePosition = GetMousePosition();
	camera->Focused = IsWindowFocused();

	camera->TargetDistance = 1;
	camera->PlayerEyesPosition = 0.5f;
	camera->ViewAngles = { 0,0 };

	camera->CameraPosition = position;
	camera->FOV.y = fovY;

	camera->ViewCamera.position = position;
	camera->ViewCamera.position.y += camera->PlayerEyesPosition;
	camera->ViewCamera.target = Vector3Add(camera->ViewCamera.position, Vector3{ 0, 0, camera->TargetDistance });
	camera->ViewCamera.up = { 0.0f, 1.0f, 0.0f };
	camera->ViewCamera.fovy = fovY;
	camera->ViewCamera.type = CAMERA_PERSPECTIVE;

	ResizeFPCameraView(camera);
	UseFPCameraMouse(camera, camera->UseMouse);
}

void UseFPCameraMouse(FPCamera* camera, bool useMouse)
{
	if (!camera)
		return;

	camera->UseMouse = useMouse;

	if (useMouse && IsWindowFocused())
		DisableCursor();
	else if (!useMouse && IsWindowFocused())
		EnableCursor();
}

void ResizeFPCameraView(FPCamera* camera)
{
	if (camera == NULL)
		return;

	float width = (float)GetScreenWidth();
	float height = (float)GetScreenHeight();

	camera->FOV.y = camera->ViewCamera.fovy;

	if (height != 0)
		camera->FOV.x = camera->FOV.y * (width / height);
}

Vector3 GetFPCameraPosition(FPCamera* camera)
{
	return camera->CameraPosition;
}

float GetSpeedForAxis(FPCamera* camera, CameraControls axis, float speed)
{
	if (camera == NULL)
		return 0;

	int key = camera->ControlsKeys[axis];
	if (key == -1)
		return 0;

	float factor = 1.0f;
	if (IsKeyDown(camera->ControlsKeys[SPRINT]))
		factor = 2;

	if (IsKeyDown(camera->ControlsKeys[axis]))
		return speed * GetFrameTime() * factor;

	return 0.0f;
}

void UpdateFPCamera(FPCamera* camera)
{
	if (camera == NULL)
		return;

	if (IsWindowFocused() != camera->Focused && camera->UseMouse)
	{
		camera->Focused = IsWindowFocused();
		if (camera->Focused)
		{
			DisableCursor();
			camera->PreviousMousePosition = GetMousePosition(); // so there is no jump on focus
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
	float direction[MOVE_DOWN + 1] = { GetSpeedForAxis(camera,MOVE_FRONT,camera->MoveSpeed.z),
									  GetSpeedForAxis(camera,MOVE_BACK,camera->MoveSpeed.z),
									  GetSpeedForAxis(camera,MOVE_RIGHT,camera->MoveSpeed.x),
									  GetSpeedForAxis(camera,MOVE_LEFT,camera->MoveSpeed.x),
									  GetSpeedForAxis(camera,MOVE_UP,camera->MoveSpeed.y),
									  GetSpeedForAxis(camera,MOVE_DOWN,camera->MoveSpeed.y) };

	mousePositionDelta.x = mousePosition.x - camera->PreviousMousePosition.x;
	mousePositionDelta.y = mousePosition.y - camera->PreviousMousePosition.y;

	camera->PreviousMousePosition = mousePosition;

	Vector3 forward = Vector3Subtract(camera->ViewCamera.target, camera->ViewCamera.position);
	forward.y = 0;
	forward = Vector3Normalize(forward);

	Vector3 right{ forward.z * -1.0f, 0, forward.x };

	Vector3 oldPostion = camera->CameraPosition;

	camera->CameraPosition = Vector3Add(camera->CameraPosition, Vector3Scale(forward, direction[MOVE_FRONT] - direction[MOVE_BACK]));
	camera->CameraPosition = Vector3Add(camera->CameraPosition, Vector3Scale(right, direction[MOVE_RIGHT] - direction[MOVE_LEFT]));

	camera->CameraPosition.y += direction[MOVE_UP] - direction[MOVE_DOWN];

	// let someone modify the projected position
	// Camera orientation calculation
	float turnRotation = GetSpeedForAxis(camera, TURN_RIGHT, camera->TurnSpeed.x) - GetSpeedForAxis(camera, TURN_LEFT, camera->TurnSpeed.x);
	float tiltRotation = GetSpeedForAxis(camera, TURN_UP, camera->TurnSpeed.y) - GetSpeedForAxis(camera, TURN_DOWN, camera->TurnSpeed.y);

	if (turnRotation != 0)
		camera->ViewAngles.x -= turnRotation * DEG2RAD;
	else if (camera->UseMouse && camera->Focused)
		camera->ViewAngles.x += (mousePositionDelta.x / -camera->MouseSensitivity);

	if (tiltRotation)
		camera->ViewAngles.y += tiltRotation * DEG2RAD;
	else if (camera->UseMouse && camera->Focused)
		camera->ViewAngles.y += (mousePositionDelta.y / -camera->MouseSensitivity);

	// Angle clamp
	if (camera->ViewAngles.y < camera->MinimumViewY * DEG2RAD)
		camera->ViewAngles.y = camera->MinimumViewY * DEG2RAD;
	else if (camera->ViewAngles.y > camera->MaximumViewY * DEG2RAD)
		camera->ViewAngles.y = camera->MaximumViewY * DEG2RAD;

	// Recalculate camera target considering translation and rotation
	Matrix translation = MatrixTranslate(0, 0, (camera->TargetDistance / 5.1f));
	Matrix rotation = MatrixRotateXYZ(Vector3{ PI * 2 - camera->ViewAngles.y, PI * 2 - camera->ViewAngles.x, 0 });
	Matrix transform = MatrixMultiply(translation, rotation);

	camera->ViewCamera.position = camera->CameraPosition;

	float eyeOfset = camera->PlayerEyesPosition;

	if (camera->ViewBobbleFreq > 0)
	{
		float swingDelta = (float)(fmax(fabs(direction[MOVE_FRONT] - direction[MOVE_BACK]), fabs(direction[MOVE_RIGHT] - direction[MOVE_LEFT])));

		camera->CurrentBobble += swingDelta * camera->ViewBobbleFreq;

		constexpr float viewBobbleDampen = 8.0f;

		eyeOfset -= sinf(camera->CurrentBobble / viewBobbleDampen) * camera->ViewBobbleMagnatude;

		camera->ViewCamera.up.x = sinf(camera->CurrentBobble / (viewBobbleDampen * 2)) * camera->ViewBobbleWaverMagnitude;
		camera->ViewCamera.up.z = -sinf(camera->CurrentBobble / (viewBobbleDampen * 2)) * camera->ViewBobbleWaverMagnitude;
	}
	else
	{
		camera->CurrentBobble = 0;
		camera->ViewCamera.up.x = 0;
		camera->ViewCamera.up.z = 0;
	}

	camera->ViewCamera.position.y += eyeOfset;

	camera->ViewCamera.target.x = camera->ViewCamera.position.x - transform.m12;
	camera->ViewCamera.target.y = camera->ViewCamera.position.y - transform.m13;
	camera->ViewCamera.target.z = camera->ViewCamera.position.z - transform.m14;
}

