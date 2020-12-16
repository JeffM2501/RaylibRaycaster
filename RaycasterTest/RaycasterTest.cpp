// RaycasterTest.cpp : Defines the entry point for the application.
//


#include "RaycasterTest.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "ResourceManager.h"
#include "MaterialManager.h"
#include "Map.h"
#include "MapRenderer.h"
#include "HudWidgets.h"
#include "MapEditorService.h"
#include "MapPicker.h"
#include "FPCamera.h"
#include "EditorGui.h"

#include <string>

GridMap::Ptr Map;
MapRenderer Renderer;
FPCamera ViewCamera;
MapVisibilitySet MainCameraViewSet(ViewCamera);

std::vector<int> SelectedCells;

Texture BackgroundImage = { 0 };

Vector2i WindowSize = { 1280,720 };

std::shared_ptr<EditorGui> Editor;

float ComputeFOV(Camera& camera)
{
    float width = (float)GetScreenWidth();
    float height = (float)GetScreenHeight();

    if (height != 0)
        return camera.fovy * (width / height);

    return 0;
}

bool CheckMapPos(FPCamera& camera, Vector3& newPostion, const Vector3& oldPostion)
{
    auto cell = Renderer.GetCell(newPostion);

    float floor = newPostion.y;

    if (cell != nullptr)
    {
        floor = cell->MapCell->Floor / 16.0f;
    }

    if (Map->CollideWithMap(Vector2{ newPostion.x, newPostion.z }, 0.1f))
    {
        newPostion = oldPostion;
        return false;
    }

    if (newPostion.y != floor)
    {
        newPostion.y = floor;
        return false;
    }
    return true;
}

void Setup()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);  // Enable Multi Sampling Anti Aliasing 4x (if available)
    InitWindow(WindowSize.x, WindowSize.y, "Raylib Raycaster test");

    ResourceManager::Setup("assets/");
    const Image& mapImage = ResourceManager::GetImage("cubicmap.png");

    float scale = 1;

    ViewCamera.UseMouseX = ViewCamera.UseMouseY = false;
    ViewCamera.ValidateMapPostion = std::bind(CheckMapPos, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    ViewCamera.Setup(scale, 45.0f, Vector3{ scale * 2, 128/16.0f, scale * 2 });
    ViewCamera.MoveSpeed.z = 4; // units per second
    ViewCamera.MoveSpeed.x = 2;
    ViewCamera.ViewBobbleFreq = 20;

    SetTargetFPS(120);

    Map = std::make_shared<GridMap>();

    Map->LoadFromFile(ResourceManager::GetAssetPath("raylib.gridmap"));

    Map->MaterialAdded = { [](size_t id, const std::string& path) {Renderer.SetupTexture(ResourceManager::GetAssetID(path)); } };

    Renderer.Setup(Map, scale);

    MapEditor::SetDefaultTextures("textures/wall/tile065.png", "textures/wall/tile098.png", "textures/wall/tile128.png");

    MapEditor::Init(Map);
    MapEditor::SetDirtyCallback([](int cell) {Renderer.BuildCellGeo(Renderer.GetCell(cell)); });

    //--------------------------------------------------------------------------------------

    WindowSize.x = GetScreenWidth();
    WindowSize.y = GetScreenHeight();

    BackgroundImage = ResourceManager::GetTexture("textures/Gradient.png");

    Editor = std::make_shared<EditorGui>(Renderer);
}

void UpdateInput()
{
    if (IsKeyPressed(KEY_F10))
        MainCameraViewSet.DrawEverything = !MainCameraViewSet.DrawEverything;

    ViewCamera.UseMouseX = ViewCamera.UseMouseY = IsMouseButtonDown(1);
    ViewCamera.Update();

    auto cell = Renderer.GetCell(ViewCamera.GetCameraPosition());
    if (cell == nullptr)
        return;
    // edit commands

    bool dirty = false;
    if (IsKeyPressed(KEY_B))
    {
        if (cell->MapCell->Floor > 0)
            MapEditor::SetCellHeights(cell->MapCell, cell->MapCell->Floor - 1, cell->MapCell->Ceiling + 1);
    }
	else if (IsKeyPressed(KEY_G))
	{
        if (cell->MapCell->Floor < 255)
            MapEditor::SetCellHeights(cell->MapCell, cell->MapCell->Floor + 1, cell->MapCell->Ceiling - 1);
	}

	if (IsKeyPressed(KEY_V))
	{
		if (cell->MapCell->Ceiling > 0)
            MapEditor::SetCellHeights(cell->MapCell, cell->MapCell->Floor, cell->MapCell->Ceiling - 1);
	}
	else if (IsKeyPressed(KEY_F))
	{
		if (cell->MapCell->Ceiling < 255)
            MapEditor::SetCellHeights(cell->MapCell, cell->MapCell->Floor, cell->MapCell->Ceiling + 1);
	}

    if (IsKeyPressed(KEY_Z) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)))
        MapEditor::Undo();

	if (IsKeyPressed(KEY_Y) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)))
		MapEditor::Redo();
}

double frameTime = 0;
void Draw3D()
{
    DrawTexturePro(BackgroundImage, Rectangle{ 0,0,(float)BackgroundImage.width,(float)BackgroundImage.height }, Rectangle{ 0,0,(float)WindowSize.x,(float)WindowSize.y }, Vector2{ 0,0 }, 0, WHITE);

    frameTime = GetTime();
    BeginMode3D(ViewCamera.GetCamera());

    Renderer.ComputeVisibility(MainCameraViewSet);
    Renderer.Draw(MainCameraViewSet);

     if (Editor != nullptr)
         Editor->Check3DViewPick(MainCameraViewSet);
  
    DrawGizmo(Vector3{ 2 * Renderer.GetDrawScale(), 0.1f * Renderer.GetDrawScale(), 2 * Renderer.GetDrawScale() });
 
     if (Editor != nullptr)
     {
         constexpr Color HighlightColor = { 255,64,64,255 };
       //  rlDisableDepthTest();
         rlDisableDepthMask();
         // draw any selected cells
         for (auto cellIndex : MapEditor::GetSelectedCells())
         {
             auto cell = Renderer.GetCell(cellIndex);
             for (auto f : cell->RenderFaces)
             {
                 Material mat = MaterialManager::GetRuntimeMaterial(f.FaceMaterial);
                 Color ogColor = mat.maps[MAP_DIFFUSE].color;
                 mat.maps[MAP_DIFFUSE].color = HighlightColor;
                 rlDrawMesh(f.FaceMesh, mat, MatrixIdentity());
                 mat.maps[MAP_DIFFUSE].color = ogColor;
             }
         }
 
         rlEnableDepthMask();
     //    rlEnableDepthTest();
     }
    EndMode3D();
    frameTime = GetTime() - frameTime;
}

void DrawHUD()
{
    int toolbarSize = 35;
    DrawFPS(10, WindowSize.y - 145);

	Vector3 pos = ViewCamera.GetCameraPosition();

	DrawMiniMap(0, toolbarSize, 8, Renderer, MainCameraViewSet);
	DrawMiniMapZoomed(0, GetScreenHeight()-125, 25, Renderer, MainCameraViewSet);
}

void DrawEditorGUI()
{
    if (Editor != nullptr)
        Editor->Show();
}

bool Update()
{
    // Main game loop

    if (IsWindowResized())
    {
        WindowSize.x = GetScreenWidth();
        WindowSize.y = GetScreenHeight();
        ViewCamera.ViewResized();
    }

    if (IsWindowReady())
    {
        UpdateInput();

        ClearBackground(BLACK);
        BeginDrawing();
        if (Editor != nullptr && Editor->EditViewMode == EditorGui::EditorModes::FPView)
        {
            Draw3D();
            DrawHUD();
        }
        else
        {
        Editor->DrawMapMode(MainCameraViewSet);
        }

        DrawEditorGUI();

        EndDrawing();
    }

    return true;
}

void Cleanup()
{
    Renderer.CleanUp();
	Map = nullptr;

	CloseWindow();
}

void Run()
{
    Setup();
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if (!Update())
            break;
    }

    Cleanup();
}
