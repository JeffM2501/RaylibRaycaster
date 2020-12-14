#pragma once
#include "MapRenderer.h"
#include "FPCamera.h"

void DrawMiniMap(int posX, int posY, int gridSize, MapRenderer& renderer, MapVisibilitySet& viewSet, bool debug = false);
void DrawMiniMapZoomed(int posX, int posY, int gridSize,  MapRenderer& renderer, MapVisibilitySet& viewSet, bool debug = false);
