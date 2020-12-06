// RaycasterTest.cpp : Defines the entry point for the application.
//


#include "RaycasterTest.h"
#include "raylib.h"
#include "raymath.h"
#include "ResourceManager.h"
#include "Map.h"
#include "MapRenderer.h"
#include "HudWidgets.h"

#include <string>

#include "raylib.h"
#include "FPCamera.h"
#include "EditorGui.h"

class Application
{
public:

    Application();
    virtual ~Application();

    void Run();

protected:
    virtual void Setup();
    virtual bool Update();
    virtual void Cleanup();

    virtual void UpdateInput();

    virtual void Draw3D();
    virtual void DrawHUD();

    bool CheckMapPos(FPCamera& camera, Vector3& newPostion, const Vector3& oldPostion);

    GridMap::Ptr Map;
    MapRenderer Renderer;
    FPCamera ViewCamera;

    Texture BackgroundImage;

    Vector2i WindowSize = { 1280,720 };

    std::shared_ptr<EditorGui> Editor;
};

void Run()
{
    Application app;
    app.Run();
}


Application::Application()
{

}

Application::~Application()
{

}

void Application::Run()
{
    Setup();
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        if (!Update())
            break;
    }

    Cleanup();
}

float ComputeFOV(Camera& camera)
{
    float width = (float)GetScreenWidth();
    float height = (float)GetScreenHeight();

    if (height != 0)
        return camera.fovy * (width / height);

    return 0;
}

void Application::Setup()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);  // Enable Multi Sampling Anti Aliasing 4x (if available)
    InitWindow(WindowSize.x, WindowSize.y, "Raylib Raycaster test");

    ResourceManager::Setup("assets/");
    const Image& mapImage = ResourceManager::GetImage("cubicmap.png");

    float scale = 1;

    ViewCamera.UseMouseX = ViewCamera.UseMouseY = false;
    ViewCamera.ValidateMapPostion = std::bind(&Application::CheckMapPos, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    ViewCamera.Setup(scale, 45.0f, Vector3{ scale * 2, 0, scale * 2 });
    ViewCamera.MoveSpeed.z = 4; // units per second
    ViewCamera.MoveSpeed.x = 2;
    ViewCamera.ViewBobbleFreq = 20;

    SetTargetFPS(120);

    Map = std::make_shared<GridMap>();

    Map->LoadFromFile(ResourceManager::GetAssetPath("raylib.gridmap"));
    Renderer.Setup(Map, scale);
    //--------------------------------------------------------------------------------------

    WindowSize.x = GetScreenWidth();
    WindowSize.y = GetScreenHeight();

    BackgroundImage = ResourceManager::GetTexture("textures/Gradient.png");

    Editor = std::make_shared<EditorGui>(Renderer, ViewCamera);
}

bool Application::CheckMapPos(FPCamera& camera, Vector3& newPostion, const Vector3& oldPostion)
{
    if (Renderer.CollideWithMap(newPostion, 0.1f))
    {
        newPostion = oldPostion;
        return false;
    }

    return true;
}

bool Application::Update()
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

        BeginDrawing();
        ClearBackground(BLACK);

        Draw3D();
        DrawHUD();

        EndDrawing();
    }
    
    return true;
}

void Application::UpdateInput()
{
    if (IsKeyPressed(KEY_F10))
        Renderer.DrawEverything = !Renderer.DrawEverything;

    ViewCamera.UseMouseX = ViewCamera.UseMouseY = IsMouseButtonDown(1);
    ViewCamera.Update();
}

double frameTime = 0;
void Application::Draw3D()
{
    DrawTexturePro(BackgroundImage, Rectangle{ 0,0,(float)BackgroundImage.width,(float)BackgroundImage.height }, Rectangle{ 0,0,(float)WindowSize.x,(float)WindowSize.y }, Vector2{ 0,0 }, 0, WHITE);

    frameTime = GetTime();
    BeginMode3D(ViewCamera.GetCamera());

    Renderer.ComputeVisibility(ViewCamera.GetCamera(), ViewCamera.GetFOVX());
    Renderer.Draw();

    DrawGizmo(Vector3{ 2 * Renderer.GetDrawScale(), 0.1f * Renderer.GetDrawScale(), 2 * Renderer.GetDrawScale() });
    EndMode3D();
    frameTime = GetTime() - frameTime;
}

void Application::DrawHUD()
{
    DrawFPS(10, WindowSize.y - 30);
// 
//     Vector3 pos = ViewCamera.GetCameraPosition();
// 
//     DrawVector3Text(&pos, WindowSize.x - 10, WindowSize.y - 30, true);
// 
//     DrawVector2Text(&ViewCamera.GetViewAngles(), WindowSize.x - 10, WindowSize.y - 50, true);
// 
//     const char* text = TextFormat("Cell%%:%.2f,Face Count:%d,Draw Time(ms):%f", ((double)Renderer.DrawnCells/(double)Renderer.MapPointer->GetCellCount()) * 100.0, Renderer.DrawnFaces, frameTime * 1000);
//     DrawText(text, WindowSize.x / 2 - 200, WindowSize.y - 30, 20, LIME);
// 
//     DrawMiniMap(0, 0, 8, Renderer,ViewCamera);
//    DrawMiniMapZoomed(WindowSize.x - (5 * 25), 0, 25, Renderer, ViewCamera);

    if (Editor != nullptr)
        Editor->Draw();
}

void Application::Cleanup()
{
    Renderer.CleanUp();
	Map = nullptr;

	CloseWindow();
}

