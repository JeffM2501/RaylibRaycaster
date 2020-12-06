#include "HudWidgets.h"


void DrawMiniMap(int posX, int posY, int gridSize, MapRenderer& renderer, const FPCamera& camera, bool debug)
{
    auto size = renderer.MapPointer->GetSize();

    int miniMapWidth = size.x * gridSize;
    int minMapHeight = size.y * gridSize;

    DrawRectangle(posX, posY, miniMapWidth, minMapHeight, GRAY);

    renderer.DoForEachCell([posX,posY,gridSize](RenderCell* cell)
        {
            int localX = cell->MapCell->Position.x * gridSize + posX;
            int localY = cell->MapCell->Position.y * gridSize + posY;

            DrawRectangle(localX + 1, localY + 1, gridSize - 1, gridSize - 1, cell->MapCell->IsSolid() ? BLACK : DARKGRAY);
        });

    renderer.DoForEachCell([posX, posY, gridSize](RenderCell* cell)
        {
            int localX = (int)(cell->MapCell->Position.x * gridSize + posX);
            int localY = (int)(cell->MapCell->Position.y * gridSize + posY);
            DrawRectangle(localX + 1, localY + 1, gridSize - 1, gridSize - 1, LIGHTGRAY);
        }, true);

    Vector2 cameraInMapSpace = camera.GetMapPosition();

    auto currentCell = renderer.GetCell(cameraInMapSpace.x, cameraInMapSpace.y);

    int localX = (int)(cameraInMapSpace.x * gridSize + posX);
    int localY = (int)(cameraInMapSpace.y * gridSize + posY);
    DrawCircle(localX, localY, gridSize / 3.0f, GREEN);

    const Camera& cam = camera.GetCamera();

    Vector2 targetVec{ cam.target.x - cam.position.x,cam.target.z - cam.position.z };
    targetVec = Vector2Normalize(targetVec);

    int targetX = localX + (int)(targetVec.x * gridSize * 2);
    int targetY = localY + (int)(targetVec.y * gridSize * 2);

    DrawLine(localX, localY, targetX, targetY, BLUE);

    Vector2 posFOV = Vector2Rotate(targetVec, camera.GetFOVX() * 0.5f);
    targetX = localX + (int)(posFOV.x * gridSize * 10);
    targetY = localY + (int)(posFOV.y * gridSize * 10);
    DrawLine(localX, localY, targetX, targetY, GREEN);

    posFOV = Vector2Rotate(targetVec, -camera.GetFOVX() * 0.5f);
    targetX = localX + (int)(posFOV.x * gridSize * 10);
    targetY = localY + (int)(posFOV.y * gridSize * 10);
    DrawLine(localX, localY, targetX, targetY, GREEN);

    if (debug)
    {
        for (auto& ray : renderer.DrawnRays)
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

        for (auto& ray : renderer.DrawnRays)
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
}

void DrawMiniMapZoomed(int posX, int posY, int gridSize, MapRenderer& renderer, const FPCamera& camera, bool debug)
{
    int miniMapWidth = 5 * gridSize;
    int minMapHeight = 5 * gridSize;

    Vector2 mapCamera = { camera.GetCameraPosition().x / renderer.GetDrawScale(), camera.GetCameraPosition().z / renderer.GetDrawScale() };

    int mapX = (int)std::floor(mapCamera.x);
    int mapY = (int)std::floor(mapCamera.y);

    Camera cam = camera.GetCamera();

    float centerX = miniMapWidth * 0.5f;
    float centerY = minMapHeight * 0.5f;
    Vector2 targetVec{ cam.target.x - cam.position.x, cam.target.z - cam.position.z };
    targetVec = Vector2Normalize(targetVec);

    Vector2 gridOffset{ mapCamera.x - mapX, mapCamera.y - mapY };
    Vector2 localOffset = Vector2Scale(gridOffset, (float)-gridSize);

    constexpr Color transColor{ 128,128,128,128 };
    constexpr Color checkedColor{ 128,255,128,128 };
    constexpr Color checkedSolidColor{ 128,128,255,128 };

    for (int y = -2; y <= +2; ++y)
    {
        for (int x = -2; x <= +2; ++x)
        {
            const RenderCell* cellPtr = renderer.GetCell(mapX + x, mapY + y);
            if (cellPtr == nullptr)
                continue;

            Vector2 gridOffset{ (float)x * gridSize, (float)y * gridSize };

            Color color = cellPtr->checkedForHit ? checkedColor : transColor;

            if (cellPtr->MapCell->IsSolid())
            {
                if (cellPtr->hitCell)
                    color = RED;
                else if (cellPtr->checkedForHit)
                    color = checkedSolidColor;
                else
                    color = BLUE;
            }

            if (cellPtr->currentCell)
                color = ORANGE;

            DrawRectangleF(posX + localOffset.x - 1 + centerX + gridOffset.x, posY + localOffset.y - 1 + centerY + gridOffset.y, gridSize - 1.0f, gridSize - 1.0f, color);
        }
    }

    DrawCircleF(posX + centerX, posY + centerY, gridSize / 6.0f, GREEN);
    DrawLineF(posX + centerX, posY + centerY, posX + centerX + (targetVec.x * gridSize), posY + centerY + (targetVec.y * gridSize), BLUE);
    Vector2 posFOV = Vector2Scale(Vector2Rotate(targetVec, camera.GetFOVX() * 0.5f), gridSize * 3.0f);
    DrawLineF(posX + centerX, posY + centerY, posX + centerX + posFOV.x, posY + centerY + posFOV.y, GREEN);

    Vector2 negFov = Vector2Scale(Vector2Rotate(targetVec, -camera.GetFOVX()* 0.5f), gridSize * 3.0f);
    DrawLineF(posX + centerX, posY + centerY, posX + centerX + negFov.x, posY + centerY + negFov.y, GREEN);
}
