#include "HudWidgets.h"
#include "MapEditorService.h"

RenderTexture VisMapTexture = RenderTexture{ 0 };

void DrawMiniMap(int screenX, int screenY, int gridSize, MapRenderer& renderer, MapVisibilitySet& viewSet, bool debug)
{
    int posX = 0;
    int posY = 0;
    auto size = renderer.MapPointer->GetSize();

    int miniMapWidth = size.x * gridSize;
    int minMapHeight = size.y * gridSize;

    constexpr Vector2 zero{ 0,0 };

	if (VisMapTexture.texture.width == 0)
	{
        VisMapTexture = LoadRenderTexture(miniMapWidth, minMapHeight);
	}

	BeginTextureMode(VisMapTexture);
	ClearBackground(DARKGRAY);

    renderer.DoForEachCell([posX,posY,gridSize](RenderCell* cell)
        {
            int localX = cell->MapCell->Position.x * gridSize + posX;
            int localY = cell->MapCell->Position.y * gridSize + posY;

            Color color = cell->MapCell->IsSolid() ? DARKBLUE : GRAY;
            if (MapEditor::CellIsSelected(cell->MapCell->Index))
                color = cell->MapCell->IsSolid() ? MAROON : RED;

            DrawRectangle(localX + 1, localY + 1, gridSize - 1, gridSize - 1, color);
        });

    renderer.DoForEachVisibleCell([posX, posY, gridSize](RenderCell* cell)
        {
            int localX = (int)(cell->MapCell->Position.x * gridSize + posX);
            int localY = (int)(cell->MapCell->Position.y * gridSize + posY);

            Color color = cell->MapCell->IsSolid() ? BLUE : LIGHTGRAY;
            if (MapEditor::CellIsSelected(cell->MapCell->Index))
                color = cell->MapCell->IsSolid() ? DARKBLUE : PINK;

            DrawRectangle(localX + 1, localY + 1, gridSize - 1, gridSize - 1, color);
        }, viewSet);

    Vector2 cameraInMapSpace = viewSet.ViewCamera.GetMapPosition();

    auto currentCell = renderer.GetCell(cameraInMapSpace.x, cameraInMapSpace.y);

    int localX = (int)(cameraInMapSpace.x * gridSize) + posX;
    int localY = (int)(cameraInMapSpace.y * gridSize) + posY;
    DrawCircle(localX, localY, gridSize / 3.0f, GREEN);

    const Camera& cam = viewSet.ViewCamera.GetCamera();

    Vector2 targetVec{ cam.target.x - cam.position.x,cam.target.z - cam.position.z };
    targetVec = Vector2Normalize(targetVec);

    int targetX = localX + (int)(targetVec.x * gridSize * 2);
    int targetY = localY + (int)(targetVec.y * gridSize * 2);

    DrawLine(localX, localY, targetX, targetY, BLUE);

    Vector2 posFOV = Vector2Rotate(targetVec, viewSet.ViewCamera.GetFOVX() * 0.5f);
    targetX = localX + (int)(posFOV.x * gridSize * 10);
    targetY = localY + (int)(posFOV.y * gridSize * 10);
    DrawLine(localX, localY, targetX, targetY, DARKGREEN);

    posFOV = Vector2Rotate(targetVec, -viewSet.ViewCamera.GetFOVX() * 0.5f);
    targetX = localX + (int)(posFOV.x * gridSize * 10);
    targetY = localY + (int)(posFOV.y * gridSize * 10);
    DrawLine(localX, localY, targetX, targetY, DARKGREEN);

    if (debug)
    {
        for (auto& ray : viewSet.DrawnRays)
        {
            RenderCell* cell = ray.Positive->Target;

            if (cell != nullptr)
            {
                int localX = (int)(cell->MapCell->Position.x * gridSize + posX);
                int localY = (int)(cell->MapCell->Position.y * gridSize + posY);
                DrawRectangle(localX + 1, localY + 1, gridSize - 1, gridSize - 1, MAROON);
            }

            cell = ray.Negative->Target;

            if (cell != nullptr)
            {
                int localX = (int)(cell->MapCell->Position.x * gridSize + posX);
                int localY = (int)(cell->MapCell->Position.y * gridSize + posY);
                DrawRectangle(localX + 1, localY + 1, gridSize - 1, gridSize - 1, GOLD);
            }
        }

        for (auto& ray : viewSet.DrawnRays)
        {
            RenderCell* cell = ray.Positive->Target;

            if (cell != nullptr)
            {
                float len = Vector2ILength(Vector2ISubtract(cell->MapCell->Position, currentCell->MapCell->Position));

                int targetX = localX + (int)(ray.Positive->Ray.x * gridSize * len);
                int targetY = localY + (int)(ray.Positive->Ray.y * gridSize * len);

                DrawLine(localX, localY, targetX, targetY, RED);
            }

            cell = ray.Negative->Target;

            if (cell != nullptr)
            {
                float len = Vector2ILength(Vector2ISubtract(cell->MapCell->Position, currentCell->MapCell->Position));

                int targetX = localX + (int)(ray.Negative->Ray.x * gridSize * len);
                int targetY = localY + (int)(ray.Negative->Ray.y * gridSize * len);

                DrawLine(localX, localY, targetX, targetY, ORANGE);
            }
        }
    }
	EndTextureMode();

	DrawTexturePro(VisMapTexture.texture,
		Rectangle{ 0,0, (float)VisMapTexture.texture.width, (float)-VisMapTexture.texture.height },
		Rectangle{ (float)screenX, (float)screenY, (float)VisMapTexture.texture.width, (float)VisMapTexture.texture.height },
        zero, 0, Color{ 255, 255, 255, 200 });
}

RenderTexture MiniMapTexture = RenderTexture{ 0 };

void DrawMiniMapZoomed(int screenX, int screenY, int gridSize, MapRenderer& renderer, MapVisibilitySet& viewSet, bool debug)
{
    int posX = 0;
    int posY = 0;
	int miniMapWidth = 5 * gridSize;
	int minMapHeight = 5 * gridSize;

    if (MiniMapTexture.texture.width == 0)
    {
        MiniMapTexture = LoadRenderTexture(miniMapWidth, minMapHeight);
    }

    BeginTextureMode(MiniMapTexture);
    ClearBackground(DARKGRAY);

    Vector2 mapCamera = { viewSet.ViewCamera.GetCameraPosition().x / renderer.GetDrawScale(), viewSet.ViewCamera.GetCameraPosition().z / renderer.GetDrawScale() };

    int mapX = (int)std::floor(mapCamera.x);
    int mapY = (int)std::floor(mapCamera.y);

    Camera cam = viewSet.ViewCamera.GetCamera();

    float centerX = miniMapWidth * 0.5f;
    float centerY = minMapHeight * 0.5f;
    Vector2 targetVec{ cam.target.x - cam.position.x, cam.target.z - cam.position.z };
    targetVec = Vector2Normalize(targetVec);

    Vector2 gridOffset{ mapCamera.x - mapX, mapCamera.y - mapY };
    Vector2 localOffset = Vector2Scale(gridOffset, (float)-gridSize);

    constexpr unsigned char alpha = 255;
    constexpr Color transColor{ 128,128,128,alpha };
    constexpr Color checkedColor{ 64,128,64,alpha };
    constexpr Color checkedSolidColor{ 128,128,255,alpha };
    constexpr Vector2 zero{ 0,0 };

    for (int y = -3; y <= +3; ++y)
    {
        for (int x = -3; x <= +3; ++x)
        {
            const RenderCell* cellPtr = renderer.GetCell(mapX + x, mapY + y);
            if (cellPtr == nullptr)
                continue;

            Vector2 gridOffset{ (float)x * gridSize, (float)y * gridSize };

            Color color = transColor;

            if (cellPtr->MapCell->IsSolid())
                color = BLUE;
            DrawRectangleF(posX + localOffset.x - 1 + centerX + gridOffset.x, posY + localOffset.y - 1 + centerY + gridOffset.y, gridSize - 1.0f, gridSize - 1.0f, color);
        }
    }

    DrawCircleF(posX + centerX, posY + centerY, gridSize / 6.0f, GREEN);
    DrawLineF(posX + centerX, posY + centerY, posX + centerX + (targetVec.x * gridSize), posY + centerY + (targetVec.y * gridSize), BLUE);
    Vector2 posFOV = Vector2Scale(Vector2Rotate(targetVec, viewSet.ViewCamera.GetFOVX() * 0.5f), gridSize * 3.0f);
    DrawLineF(posX + centerX, posY + centerY, posX + centerX + posFOV.x, posY + centerY + posFOV.y, GREEN);

    Vector2 negFov = Vector2Scale(Vector2Rotate(targetVec, -viewSet.ViewCamera.GetFOVX()* 0.5f), gridSize * 3.0f);
    DrawLineF(posX + centerX, posY + centerY, posX + centerX + negFov.x, posY + centerY + negFov.y, GREEN);

    int dataFontSize = 20;

    // draw current cell data

    // position
    DrawText(TextFormat("X:%d Y:%d", mapX, mapY),0,minMapHeight- dataFontSize, dataFontSize,RAYWHITE);

    // height info
    const RenderCell* cellPtr = renderer.GetCell(mapX, mapY);
    DrawText(TextFormat("F:%d C:%d", cellPtr->MapCell->Floor, cellPtr->MapCell->Ceiling), 0, minMapHeight - dataFontSize * 2, dataFontSize, RAYWHITE);

    EndTextureMode();

    DrawTexturePro(MiniMapTexture.texture,
        Rectangle{ 0,0, (float)MiniMapTexture.texture.width, (float)-MiniMapTexture.texture.height },
        Rectangle{ (float)screenX, (float)screenY, (float)MiniMapTexture.texture.width, (float)MiniMapTexture.texture.height },
        zero, 0, WHITE);
}
