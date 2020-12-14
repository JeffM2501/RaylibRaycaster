#pragma once

#include "Map.h"
#include "MapRenderer.h"

namespace MapPicker
{ 
	class PickResults
	{
	public:
		int CellIndex = -1;
		Directions FaceDirection = Directions::YNeg;
		Vector3 HitPosition = { 0 };
		Ray	PickRay;
	};

	PickResults PickFace(MapRenderer& renderer, MapVisibilitySet viewSet, Vector2 screenPoint);
}
