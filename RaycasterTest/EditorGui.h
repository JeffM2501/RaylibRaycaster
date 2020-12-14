#pragma once
#include "raygui.h"
#include "ricons.h"

#include "MapRenderer.h"
#include "FPCamera.h"

class EditorGui
{
public:
	EditorGui(MapRenderer& renderer, FPCamera& camera);

	void Draw();

    enum class EditorModes
    {
        FPView,
        MapView,
    };

    EditorModes EditViewMode = EditorModes::FPView;

    void DrawMapMode();

    void Check3DViewPick(MapVisibilitySet& viewSet);

protected:
	void ShowToolbar();

private:
	MapRenderer& Renderer;
	FPCamera& ViewCamera;
};