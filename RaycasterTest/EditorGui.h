#pragma once
#include "raygui.h"
#include "ricons.h"

#include "MapRenderer.h"
#include "FPCamera.h"

class EditorGui
{
public:
	EditorGui(MapRenderer& renderer);

	void Show();

    enum class EditorModes
    {
        FPView,
        MapView,
    };

    EditorModes EditViewMode = EditorModes::FPView;

    void DrawMapMode(MapVisibilitySet& viewSet);

    void Check3DViewPick(MapVisibilitySet& viewSet);

    int ToolbarHeight = 34;

protected:
	void ShowToolbar();

    void SetViewMode(EditorModes mode);

    void HandleDrag();
    void HandleSelection();
    void DrawMap();
    void DrawSelection(MapVisibilitySet& viewSet);

    bool ValidClickPoint(const Vector2& position) const;
    Vector2 MouseToMap(Vector2 position) const;
    std::vector<int> GetCellsInRect(const Vector2& min, const Vector2& max);

private:
	MapRenderer& Renderer;

    int MapGridSize = 25;
	
    Camera2D MapViewCamera;
    int ZoomTicks = 3;
    float ZoomLevels[7] = { 4, 2, 1.5f, 1, 0.5f, 0.25f, 0.125f };

    bool InPanDrag = false;
    bool InSelectDrag = false;
    Vector2 LastDragPos{ 0, 0 };

    std::vector<int> HoverSelectCells;
};