#include "EditorGui.h"

#include "ResourceManager.h"
#include "MapEditorService.h"
#include "MapPicker.h"
#include "MapRenderer.h"

#define GUI_PROPERTY_LIST_IMPLEMENTATION
#include "PropertyList.h"

std::function<void(void)> ActivePopupFunction;
std::string ActivePopup;

void OpenPopup(const std::string& popup)
{
    if (ActivePopup.empty())
        ActivePopup = popup;
}

bool ShowPopup(const std::string& popup, std::function<void(void)> func)
{
    if (ActivePopup == popup)
    {
        ActivePopupFunction = func;
        return true;
    }
    
    return false; 
}

void ClosePopup()
{
    ActivePopup.clear();
    ActivePopupFunction = nullptr;
}

bool PopupActive()
{
    return !ActivePopup.empty();
}

void EndPopup()
{
    GuiLock();
}

void ApplyPopup()
{
    if (PopupActive())
    {
        GuiUnlock();
        ActivePopupFunction();
        GuiLock();
    }
}

bool ShiftIsDown()
{
    return IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
}

bool ConditionalButton(Rectangle rect, const char* text, bool enabled)
{
    if (enabled)
        return GuiButton(rect, text);

    GuiDummyRec(rect, text);
    return false;
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
    if (ConditionalButton(buttonRect, GuiIconText(RICON_UNDO_FILL, nullptr), MapEditor::CanUndo()))
         MapEditor::Undo();
 
    headerOffset += 32;

    buttonRect.x = headerOffset;
    if (ConditionalButton(buttonRect, GuiIconText(RICON_REDO_FILL, nullptr), MapEditor::CanRedo()))
        MapEditor::Redo();

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
}

void EditorGui::ShowSidebar()
{
    float xStartPos = (float)GetScreenWidth() - SidebarWidth;
    float height = (float)GetScreenHeight() - ToolbarHeight;
    GuiPanel(Rectangle{ xStartPos, (float)ToolbarHeight, (float)SidebarWidth, height });

    // tabs
    float headerOffset = xStartPos + 2;
    if (GuiToggle(Rectangle{ headerOffset, ToolbarHeight,60,30 }, GuiIconText(RICON_TARGET_MOVE_FILL, "Edit"), ActiveTab == ActiveTabs::Edit))
        ActiveTab = ActiveTabs::Edit;
    headerOffset += 62;
    if (GuiToggle(Rectangle{ headerOffset, ToolbarHeight,60,30 }, GuiIconText(RICON_BRUSH_PAINTER, "Paint"), ActiveTab == ActiveTabs::Paint))
        ActiveTab = ActiveTabs::Paint;
    headerOffset += 62;
    if (GuiToggle(Rectangle{ headerOffset, ToolbarHeight,70,30 }, GuiIconText(RICON_PLAYER, "Items"), ActiveTab == ActiveTabs::Items))
        ActiveTab = ActiveTabs::Items;
    headerOffset += 72;

    float yOffset = ToolbarHeight + 32;
    headerOffset = 4;
    Rectangle bounds{ xStartPos + 2, yOffset, SidebarWidth - 2, (float)GetScreenHeight() - yOffset - 2 };
    GuiPanel(bounds);
    
    switch (ActiveTab)
    {
    case EditorGui::ActiveTabs::Edit:
        ShowEditPanel(bounds);
        break;
    case EditorGui::ActiveTabs::Paint:
        ShowPaintPanel(bounds);
        break;
    case EditorGui::ActiveTabs::Items:
        ShowItemPanel(bounds);
        break;
    default:
        break;
    }
}

enum class SelectionSolidTypes
{
    Solid,
    Open,
    Mixed,
};

void EditorGui::ShowEditPanel(const Rectangle& bounds)
{
    auto selectedCells = MapEditor::GetSelectedCells();
    bool selectionValid = selectedCells.size() > 0;

    float selectedFloorHeight = -1;
    float selectedCeilingHeight = -1;

    SelectionSolidTypes selectionSolidType = SelectionSolidTypes::Mixed;

    
    if (selectionValid)
    {
        bool isSolid = Renderer.GetCell(selectedCells[0])->MapCell->IsSolid();

        selectionSolidType = isSolid ? SelectionSolidTypes::Solid: SelectionSolidTypes::Open;;

        for (int index : selectedCells)
        {
            auto cell = Renderer.GetCell(index);

            if (cell->MapCell->IsSolid() != isSolid)
                selectionSolidType = SelectionSolidTypes::Mixed;

            float f = cell->GetFloorValue();
            float c = cell->GetCeilingValue();

            if (selectedFloorHeight < 0)
                selectedFloorHeight = f;
            else if (selectedFloorHeight != f)
            {
                selectedFloorHeight = -1;
                selectedCeilingHeight = -1;
                break;
            }

            if (selectedCeilingHeight < 0)
                selectedCeilingHeight = c;
            else if (selectedCeilingHeight != c)
            {
                selectedFloorHeight = -1;
                selectedCeilingHeight = -1;
                break;
            }
        }
    }

    float headerOffset = bounds.x+2;
    float yOffset = bounds.y + 2;

    Rectangle buttonRect{ headerOffset, yOffset, 60, 30 };

    GuiLabel(buttonRect, "Floor");
    buttonRect.x += 62;
    buttonRect.width = 30;
    if (ConditionalButton(buttonRect, GuiIconText(RICON_ARROW_BOTTOM_FILL, nullptr), selectionValid))
    {
        MapEditor::IncrementCellHeights(nullptr, -1, 1);
    }

    buttonRect.x += 32;
    if (ConditionalButton(buttonRect, GuiIconText(RICON_ARROW_TOP_FILL, nullptr), selectionValid))
    {
        MapEditor::IncrementCellHeights(nullptr, 1, -1);
    }
    buttonRect.x += 34;
    buttonRect.width = 60;

    if (selectionValid)
    {
        if (GuiButton(buttonRect, selectedFloorHeight < 0 ? "..." : TextFormat("%.3f", selectedFloorHeight)))
            OpenPopup("floordepth");
    }
    
    ShowPopup("floordepth",[this, selectedFloorHeight, buttonRect]()
        {
            Rectangle windowRect = { buttonRect.x + buttonRect.width - 200, buttonRect.y, 200, 64 };
            if (!GuiWindowBox(windowRect, "Set Floor Height"))
            {
                Rectangle sliderRect = { windowRect.x + 50,windowRect.y+30,windowRect.width-100,30 };

                float oldVal = selectedFloorHeight;
                if (oldVal < 0)
                    oldVal = Renderer.GetCell(MapEditor::GetSelectedCells()[0])->GetFloorValue();
 
                float newValue = GuiSlider(sliderRect, "Height", TextFormat("%.2f", oldVal), oldVal, 0, 255 / 16.0f);
                if (newValue != oldVal)
                    MapEditor::SetCellFloors(nullptr, (uint8_t)(newValue * 16));
            }
            else
            {
                ClosePopup();
            }
            EndPopup();
        });

    yOffset += 32;
    buttonRect = { headerOffset, yOffset, 60, 30 };
    GuiLabel(buttonRect, "Ceiling");
    buttonRect.x += 62;
    buttonRect.width = 30;
    if (ConditionalButton(buttonRect, GuiIconText(RICON_ARROW_BOTTOM_FILL, nullptr), selectionValid))
    {
        MapEditor::IncrementCellHeights(nullptr, 0, -1);
    }

    buttonRect.x += 32;
    if (ConditionalButton(buttonRect, GuiIconText(RICON_ARROW_TOP_FILL, nullptr), selectionValid))
    {
        MapEditor::IncrementCellHeights(nullptr, 0, 1);
    }
    buttonRect.x += 34;
    buttonRect.width = 60;

    if (selectionValid)
    {
        if (GuiButton(buttonRect, selectedCeilingHeight < 0 ? "..." : TextFormat("%.3f", selectedCeilingHeight)))
            OpenPopup("ceilingdepth");
    }

    ShowPopup("ceilingdepth", [this, selectedFloorHeight, selectedCeilingHeight, buttonRect]()
        {
            Rectangle windowRect = { buttonRect.x + buttonRect.width - 200, buttonRect.y, 200, 64 };
            if (!GuiWindowBox(windowRect, "Set Ceiling Height"))
            {
                Rectangle sliderRect = { windowRect.x + 50,windowRect.y + 30,windowRect.width - 100,30 };

                float oldFloor = selectedFloorHeight;
                if (oldFloor < 0)
                    oldFloor = Renderer.GetCell(MapEditor::GetSelectedCells()[0])->GetFloorValue();

                float oldVal = selectedCeilingHeight;
                if (oldVal < 0)
                    oldVal = Renderer.GetCell(MapEditor::GetSelectedCells()[0])->GetCeilingValue();

                oldVal = oldVal - oldFloor;

                float newValue = GuiSlider(sliderRect, "Height", TextFormat("%.2f", oldVal), oldVal, 0, 255 / 16.0f);
                if (newValue != oldVal)
                    MapEditor::SetCellCeilings(nullptr, (uint8_t)(newValue * 16));
            }
            else
            {
                ClosePopup();
            }
            EndPopup();
        });

    // solid/open state
    yOffset += 34;
    buttonRect = { headerOffset, yOffset, 30, 30 };

    // checkbox
    if (selectionSolidType == SelectionSolidTypes::Mixed)
    {
        GuiLabel(buttonRect, "...");
    }
    else
    {
        bool checked = GuiCheckBox(buttonRect, "Solid", selectionSolidType == SelectionSolidTypes::Solid);
        if (checked != (selectionSolidType == SelectionSolidTypes::Solid))
        {
            MapEditor::SetCellSolid(nullptr, checked);
        }
    }

    yOffset += 34;
    buttonRect = { headerOffset, yOffset, 30, 30 };
    
    GuiEnableTooltip();
    // paint tools


    GuiSetTooltip("No Paint");
    if (GuiToggle(buttonRect, GuiIconText(RICON_ZOOM_CENTER, nullptr), PaintToolMode == PaintToolModes::None))
    {
        PaintToolMode = PaintToolModes::None;
    }
    buttonRect.x += 32;

    GuiSetTooltip("Paint Solid");
    if (GuiToggle(buttonRect, GuiIconText(RICON_ZOOM_ALL, nullptr), PaintToolMode == PaintToolModes::Solid))
    {
        PaintToolMode = PaintToolModes::Solid;
    }
    buttonRect.x += 32;

    GuiSetTooltip("Paint Solid");
    if (GuiToggle(buttonRect, GuiIconText(RICON_BOX_DOTS_BIG, nullptr), PaintToolMode == PaintToolModes::Open))
    {
        PaintToolMode = PaintToolModes::Open;
    }

    GuiClearTooltip();
    GuiDisableTooltip();
}

void EditorGui::ShowPaintPanel(const Rectangle& bounds)
{
    bool selectionValid = MapEditor::GetSelectedCells().size() > 0;
}

void EditorGui::ShowItemPanel(const Rectangle& bounds)
{

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
    if (PopupActive())
        GuiLock();

    ShowToolbar();
    ShowSidebar();

    ApplyPopup();

    GuiUnlock();
}

bool EditorGui::ValidClickPoint(const Vector2& position) const
{
    return position.y >= ToolbarHeight && position.x <= (GetScreenWidth() - SidebarWidth);
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

std::vector<int> EditorGui::GetCellsInRect(const Vector2& min, const Vector2& max)
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

    if (PaintToolMode == PaintToolModes::None)
    {
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
    else if (PaintToolMode == PaintToolModes::Solid && IsMouseButtonDown(0))
    {
        Vector2 cellPos = MouseToMap(mousePos);
        auto cell = Renderer.GetCell(cellPos.x,cellPos.y);
        if (cell == nullptr)
            return;

        MapEditor::SetCellSolid(cell->MapCell, true);
    }
    else if (PaintToolMode == PaintToolModes::Open && IsMouseButtonDown(0))
    {
        Vector2 cellPos = MouseToMap(mousePos);
        auto cell = Renderer.GetCell(cellPos.x, cellPos.y);
        if (cell == nullptr)
            return;

        MapEditor::SetCellSolid(cell->MapCell, false);
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

    if (!ValidClickPoint(pos))
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
