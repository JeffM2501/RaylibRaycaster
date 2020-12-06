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

protected:
	void ShowToolbar();

private:
	MapRenderer& Renderer;
	FPCamera& ViewCamera;
};