#pragma once
#include "raygui.h"
#include "ricons.h"

#include "MapRenderer.h"
#include "FPCamera.h"

class EditorGui
{
public:
	EditorGui(MapRenderer& renderer);

    void AddTextureFolder(const std::string& path);

	void Show();

    enum class EditorModes
    {
        FPView,
        MapView,
    };

    EditorModes EditViewMode = EditorModes::FPView;

    enum class PaintToolModes
    {
        None,
        Solid,
        Open,
        FaceTexture,
    };

    PaintToolModes PaintToolMode = PaintToolModes::None;

    void DrawMapMode(MapVisibilitySet& viewSet);

    void Check3DViewPick(MapVisibilitySet& viewSet);

    float ToolbarHeight = 34;
    float SidebarWidth = 250;

protected:
	void ShowToolbar();
    void ShowSidebar();

    void SetViewMode(EditorModes mode);

    void HandleDrag();
    void HandleSelection();
    void DrawMap();
    void DrawSelection(MapVisibilitySet& viewSet);

    bool ValidClickPoint(const Vector2& position) const;
    Vector2 MouseToMap(Vector2 position) const;
    std::vector<int> GetCellsInRect(const Vector2& min, const Vector2& max);

    enum class ActiveTabs
    {
        Edit,
        Paint,
        Items,
    };

    ActiveTabs ActiveTab = ActiveTabs::Edit;

    void ShowEditPanel(const Rectangle& bounds);
    void ShowPaintPanel(const Rectangle& bounds);
    void ShowItemPanel(const Rectangle& bounds);

    std::vector<size_t> TextureCache;
    int VissibleTextureIndex = 0;
    int SelectedTextureIndex = -1;

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