// RaycasterTest.cpp : Defines the entry point for the application.
//


#include "RaycasterTest.h"
#include "raylib.h"
#include "raymath.h"
#include "ResourceManager.h"
#include "Map.h"

#include <string>

#include "raylib.h"

float ComputeFOV(Camera& camera)
{
	float width = (float)GetScreenWidth();
	float height = (float)GetScreenHeight();

    if (height != 0)
        return camera.fovy * (width / height)  * 0.5f;

    return 0;
}

void DoMain()
{
    // Initialization
      //--------------------------------------------------------------------------------------
    int screenWidth = 1920;
    int screenHeight = 980;
   
    Color textColor(LIGHTGRAY);
    InitWindow(screenWidth, screenHeight, "Raylib Raycaster test");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    ResourceManager::Setup("assets/");
    const Image& mapImage = ResourceManager::GetImage("cubicmap.png");

    float scale = 1;

	Camera camera;
 	camera.position = { scale * 2, scale / 2, scale * 2 };
 	camera.target = { 0.0f, scale / 2, 1.0f };
 	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.fovy = 60.0f;
	camera.type = CAMERA_PERSPECTIVE;
    float fovX = ComputeFOV(camera);

    SetCameraMode(camera, CAMERA_FIRST_PERSON); // Set a first person camera mode

    SetTargetFPS(60);

    MapRenderer renderer;
    CastMap::Ptr map = std::make_shared<CastMap>();

    size_t floorID = renderer.SetupTexture(ResourceManager::GetAssetID("textures/wall/tile098.png"));
	size_t ceilingID = renderer.SetupTexture(ResourceManager::GetAssetID("textures/wall/tile128.png"));
	size_t wallID = renderer.SetupTexture(ResourceManager::GetAssetID("textures/wall/tile065.png"));

    map->LoadFromImage(mapImage, scale, wallID, floorID, ceilingID);

    renderer.Setup(map, scale);
    //--------------------------------------------------------------------------------------

    int width = GetScreenWidth();
    int height = GetScreenHeight();
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if (IsWindowResized())
        {
            width = GetScreenWidth();
            height = GetScreenHeight();
            fovX = ComputeFOV(camera);
        }

        if (IsWindowReady())
        {
            Vector3 pos = camera.position;
            UpdateCamera(&camera);                  // Update camera
            if (!renderer.PointIsOpen(camera.position, scale * 0.1f))
                camera.position = pos;

            // Draw
            //----------------------------------------------------------------------------------
            BeginDrawing();

            ClearBackground(BLACK);

			BeginMode3D(camera);

            renderer.ComputeVisibility(camera, fovX);
            renderer.Draw();

            Vector3 camVec = Vector3Normalize(Vector3Subtract(camera.target, camera.position));

            DrawGizmo(Vector3{ 2 * scale, 0.1f * scale, 2 * scale });
			EndMode3D();

            DrawFPS(10, height - 30);
            DrawVector3Text(&camera.position, width-10, height-30, true);

            renderer.DrawMiniMap(0, 0, 5, camera, fovX);
            renderer.DrawMiniMapZoomed(width - (5 * 25), 0, 25, camera, fovX);

            EndDrawing();
        }
        //----------------------------------------------------------------------------------
    }

    renderer.CleanUp();
	map = nullptr;

	CloseWindow();
}

