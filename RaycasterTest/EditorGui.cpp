#include "EditorGui.h"

#include "ResourceManager.h"
#include "MapEditorService.h"
#include "MapPicker.h"
#include "MapRenderer.h"

bool ShiftIsDown()
{
    return IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
}

EditorGui::EditorGui(MapRenderer& renderer) : Renderer(renderer)
{
	GuiLoadStyle(ResourceManager::GetAssetPath("editor/themes/ashes.rgs").c_str());
    MapViewCamera.offset = Vector2{ 0, (float)ToolbarHeight };
    MapViewCamera.rotation = 0;
    MapViewCamera.target = Vector2{ 0,0 };
    MapViewCamera.zoom = 1;
}

void EditorGui::ShowToolbar()
{
    float width = (float)GetScreenWidth();
    float middle = (width * 0.5f) - 100;
    GuiPanel(Rectangle{ 0, 0, width, (float)ToolbarHeight });

    float headerOffset = 2;
    //file IO
    GuiButton(Rectangle{ headerOffset, 2, 60, 30 }, GuiIconText(RICON_FILE_NEW, "New"));
    headerOffset += 60 + 2;
    GuiButton(Rectangle{ headerOffset, 2, 60, 30 }, GuiIconText(RICON_FILE_OPEN, "Open"));
    headerOffset += 60 + 2;
    GuiButton(Rectangle{ headerOffset, 2, 60, 30 }, GuiIconText(RICON_FILE_SAVE, "Save"));
    headerOffset += 60 + 2;

    headerOffset += 32;

    Rectangle buttonRect{ headerOffset, 2, 30, 30 };
    // undo/redo
    if (MapEditor::CanUndo())
    {
        if (GuiButton(buttonRect, GuiIconText(RICON_UNDO_FILL, nullptr)))
            MapEditor::Undo();
    }
    else
    {
        GuiDummyRec(buttonRect, GuiIconText(RICON_UNDO_FILL, nullptr));
    }
    headerOffset += 32;

    buttonRect.x = headerOffset;
    if (MapEditor::CanRedo())
    {
        if (GuiButton(buttonRect, GuiIconText(RICON_REDO_FILL, nullptr)))
            MapEditor::Redo();
    }
    else
    {
        GuiDummyRec(buttonRect, GuiIconText(RICON_REDO_FILL, nullptr));
    }

    // view modes
    if (GuiToggle(Rectangle{ middle + 2, 2, 30, 30 }, GuiIconText(RICON_MODE_3D, nullptr), EditViewMode == EditorModes::FPView))
        SetViewMode(EditorModes::FPView);

    if (GuiToggle(Rectangle{ middle + 34, 2, 30, 30 }, GuiIconText(RICON_MODE_2D, nullptr), EditViewMode == EditorModes::MapView))
        SetViewMode(EditorModes::MapView);

    if (EditViewMode == EditorModes::MapView && GuiButton(Rectangle{ middle + 66, 2, 30, 30 }, GuiIconText(RICON_TARGET_BIG_FILL, nullptr)))
    {
        MapViewCamera.offset.x = 0;
        MapViewCamera.offset.y = (float)ToolbarHeight;
    }

    if (GuiButton(Rectangle{ middle + 64, 2, 70, 30 }, "Test"))
    {
        MapEditor::IncrementCellHeights(nullptr, -1, +1);
    }
}

void EditorGui::SetViewMode(EditorModes mode)
{
    if (EditViewMode == mode)
        return;

    EditViewMode = mode;
    InPanDrag = false;
}

void EditorGui::Show()
{
    ShowToolbar();
}

bool EditorGui::ValidClickPoint(const Vector2& position) const
{
    return position.y >= ToolbarHeight;
}

void EditorGui::HandleDrag()
{
    float wheel = GetMouseWheelMove();
    if (wheel != 0)
    {
        float increment = 1.25f;
        if (wheel < 0)
            ++ZoomTicks;
        else
            --ZoomTicks;

        if (ZoomTicks < 0)
            ZoomTicks = 0;
        else if (ZoomTicks >= 7)
            ZoomTicks = 6;

        MapViewCamera.zoom = ZoomLevels[ZoomTicks];
    }

    if (InSelectDrag)
        return;

    if (!InPanDrag)
    {
        if (IsMouseButtonDown(1))
        {
            InPanDrag = true;
            LastDragPos = GetMousePosition();
        }
    }
    else if (!IsMouseButtonDown(1))
        InPanDrag = false;

    if (InPanDrag)
    {
        Vector2 pos = GetMousePosition();
        Vector2 delta = Vector2Subtract(pos, LastDragPos);

        MapViewCamera.offset.x += delta.x;
        MapViewCamera.offset.y += delta.y;

        LastDragPos = pos;
    }
}

Vector2 EditorGui::MouseToMap(Vector2 position) const
{
    position.x -= MapViewCamera.offset.x;
    position.y -= MapViewCamera.offset.y;
    position.x /= (float)MapGridSize;
    position.y /= (float)MapGridSize;

    return position;
}

std::vector<int>  EditorGui::GetCellsInRect(const Vector2& min, const Vector2& max)
{
    std::vector<int> cells;

    for (int y = (int)min.y; y <= max.y; ++y)
    {
        for (int x = (int)min.x; x <= max.x; ++x)
        {
            auto cell = Renderer.GetCell(x, y);
            if (cell != nullptr)
            {
                cells.push_back(cell->MapCell->Index);
            }
        }
    }
    return cells;
}

void EditorGui::HandleSelection()
{
    HoverSelectCells.clear();

    if (InPanDrag)
    {
       
        InSelectDrag = false;
        return;
    }

    Vector2 mousePos = GetMousePosition();

    if (InSelectDrag)
    {
        Vector2 p1 = MouseToMap(LastDragPos);
        Vector2 p2 = MouseToMap(mousePos);

        Vector2 min = { std::min(p1.x,p2.x),std::min(p1.y,p2.y) };
        Vector2 max = { std::max(p1.x,p2.x),std::max(p1.y,p2.y) };

        std::vector<int> cells = GetCellsInRect(min, max);

        if (!IsMouseButtonDown(0))
        {
            MapEditor::SelectCells(cells, ShiftIsDown());
            InSelectDrag = false;
        }
        else
        {
            HoverSelectCells = cells;
        }
    }
    else
    {
        if (IsMouseButtonDown(0) && ValidClickPoint(mousePos))
        {
            InSelectDrag = true;
            LastDragPos = mousePos;
        }
    }
}

void EditorGui::DrawMap()
{
    DrawRectangle(-1, -1, (Renderer.MapPointer->GetSize().x * MapGridSize) + 2, ((Renderer.MapPointer->GetSize().y) * MapGridSize) + 2, DARKGRAY);
    Renderer.DoForEachCell([this](RenderCell* cell)
        {
            Color color = cell->MapCell->IsSolid() ? DARKBLUE : WHITE;

            int localX = (cell->MapCell->Position.x * MapGridSize) - 1;
            int localY = (cell->MapCell->Position.y * MapGridSize) - 1;

            if (!cell->MapCell->IsSolid())
            {
                auto& texture = ResourceManager::GetTexture(Renderer.MapPointer->MaterialList[cell->MapCell->CellTextures[Directions::YNeg]]);
                DrawTexturePro(texture, Rectangle{ 0, 0,(float)texture.width,(float)texture.height },
                                        Rectangle{ (float)localX, (float)localY, (float)MapGridSize, (float)MapGridSize },
                                        Vector2{ 0, 0 }, 0, WHITE);
                DrawRectangleLines(localX, localY, MapGridSize, MapGridSize, GRAY);
            }
            else
                DrawRectangle(localX + 1, localY + 1, MapGridSize - 1, MapGridSize - 1, color);
        });
}

void EditorGui::DrawSelection(MapVisibilitySet& viewSet)
{
    for (auto index : MapEditor::GetSelectedCells())
    {
        auto cell = Renderer.GetCell(index);

        int localX = (cell->MapCell->Position.x * MapGridSize) - 1;
        int localY = (cell->MapCell->Position.y * MapGridSize) - 1;

        Color color = cell->MapCell->IsSolid() ? MAROON : RED;

        DrawRectangle(localX, localY, MapGridSize, MapGridSize, color);
        DrawRectangleLines(localX, localY, MapGridSize, MapGridSize, MAROON);
    }

    for (auto index : HoverSelectCells)
    {
        auto cell = Renderer.GetCell(index);

        int localX = (cell->MapCell->Position.x * MapGridSize) - 1;
        int localY = (cell->MapCell->Position.y * MapGridSize) - 1;

        Color color = cell->MapCell->IsSolid() ? GOLD : YELLOW;

        DrawRectangle(localX, localY, MapGridSize, MapGridSize, color);
        DrawRectangleLines(localX, localY, MapGridSize, MapGridSize, MAROON);
    }
}


void EditorGui::DrawMapMode(MapVisibilitySet& viewSet)
{
    HandleDrag();

    ClearBackground(BLACK);

    HandleSelection();

    BeginMode2D(MapViewCamera);
    DrawMap();
    DrawSelection(viewSet);
    EndMode2D();


    if (InSelectDrag)
    {
        auto mousePos = GetMousePosition();

        DrawRectangleLines((int)(LastDragPos.x), (int)(LastDragPos.y), (int)(mousePos.x - LastDragPos.x),(int)(mousePos.y - LastDragPos.y), RED);
    }

    DrawText(TextFormat("Zoom: %.2f%%", MapViewCamera.zoom * 100), 0, GetScreenHeight()-20, 20, GREEN);
}

void EditorGui::Check3DViewPick(MapVisibilitySet& viewSet)
{
    if (EditViewMode == EditorModes::MapView)
        return;

    auto pos = GetMousePosition();

    if (pos.y < ToolbarHeight)
        return;

    // check for picking
    if (IsMouseButtonPressed(0))
    {
        bool add = ShiftIsDown();

        auto results = MapPicker::PickFace(Renderer, viewSet, pos);
        if (results.CellIndex >= 0)
            MapEditor::SelectCell(results.CellIndex,add);
    }
}
