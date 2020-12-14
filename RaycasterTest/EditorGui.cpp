#include "EditorGui.h"

#include "ResourceManager.h"
#include "MapEditorService.h"
#include "MapPicker.h"
#include "MapRenderer.h"

EditorGui::EditorGui(MapRenderer& renderer, FPCamera& camera) : Renderer(renderer),ViewCamera(camera)
{
	GuiLoadStyle(ResourceManager::GetAssetPath("editor/themes/ashes.rgs").c_str());
}

void EditorGui::ShowToolbar()
{
    GuiPanel(Rectangle{ 0,0,(float)GetScreenWidth(),34 });
    GuiButton(Rectangle{ 2, 2, 60, 30 }, GuiIconText(RICON_FILE_NEW, "New"));
    GuiButton(Rectangle{ 64, 2, 60, 30 }, GuiIconText(RICON_FILE_OPEN, "Open"));
    GuiButton(Rectangle{ 126, 2, 60, 30 }, GuiIconText(RICON_FILE_SAVE, "Save"));
}

void EditorGui::Draw()
{
    ShowToolbar();
}

void EditorGui::DrawMapMode()
{

}

void EditorGui::Check3DViewPick(MapVisibilitySet& viewSet)
{
    if (EditViewMode == EditorModes::MapView)
        return;

    // check for picking
    if (IsMouseButtonPressed(0))
    {
        bool add = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT));

        auto results = MapPicker::PickFace(Renderer, viewSet, GetMousePosition());
        if (results.CellIndex >= 0)
            MapEditor::SelectCell(results.CellIndex,add);
    }
}
