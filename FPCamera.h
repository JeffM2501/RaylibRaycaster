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

#ifndef FP_CAMERA_H
#define FP_CAMERA_H

#include "raylib.h"
#include "raymath.h"

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

typedef struct  
{
	// keys used to control the camera
	int ControlsKeys[LAST_CONTROL];

	// the speed in units/second to move 
	// X = sidestep
	// Y = jump/fall
	// Z = forward
	Vector3 MoveSpeed;

	// the speed for turning when using keys to look
	// degrees/second
	Vector2 TurnSpeed;

	// use the mouse for looking?
	bool UseMouse;

	// how many pixels equate out to an angle move, larger numbers mean slower, more accurate mouse
	float MouseSensitivity;

	// how far down can the camera look
	float MinimumViewY;

	// how far up can the camera look
	float MaximumViewY;

	// how fast the view should bobble as you move
	// defaults to 0 for no bobble
	float ViewBobbleFreq;

	// how high up/down will the bobble be
	float ViewBobbleMagnatude;

	// how far left and right should the view bobble
	float ViewBobbleWaverMagnitude;

	// the position of the base of the camera (on the floor)
	// note that this will not be the view position because it is offset by the eye height.
	// this value is also not changed by the view bobble
	Vector3 CameraPosition;

	// how far from the base of the camera is the player's view
	float PlayerEyesPosition;

	// the Raylib camera to pass to raylib view functions.
	Camera ViewCamera;

	// the field of view in X and Y
	Vector2 FOV;

	// state for mouse movement
	Vector2 PreviousMousePosition;

	// state for view movement
	float TargetDistance; 

	// state for view angles
	Vector2 ViewAngles; 

	// state for bobble
	float CurrentBobble;

	// state for window focus
	bool Focused;
}FPCamera;

// called to initialize a camera to default values
void InitFPCamera(FPCamera *camera, float fovY, Vector3 position);

// called to update field of view in X when window resizes
void ResizeFPCameraView(FPCamera* camera);

// turn the use of mouselook on/off, also updates the cursor visibility
void UseFPCameraMouse(FPCamera* camera, bool useMouse);

// Get the camera's postion in world (or game) space
Vector3 GetFPCameraPosition(FPCamera* camera);

// update the camera for the current frame
void UpdateFPCamera(FPCamera* camera);

#endif //FP_CAMERA_H
