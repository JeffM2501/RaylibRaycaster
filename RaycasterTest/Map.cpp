#include "Map.h"
#include "ResourceManager.h"
#include "raymath.h"

#include <algorithm>
#include <cmath>

GridMap::~GridMap()
{
}

bool GridMap::IsSolid(int x, int y, Color* imageData)
{
	return (imageData + (y * Size.x + x))->b > 0;
}

void GridMap::LoadFromImage(const Image& image, float scale, size_t walls, size_t floor, size_t ceiling)
{
	Size.x = image.width;
	Size.y = image.height;

	Cells.clear();

	Cells.resize(size_t(Size.x) * size_t(Size.y));
	Color* imageData = GetImageData(image);

	for (int y = 0; y < Size.y; y++)
	{
		for (int x = 0; x < Size.x; x++)
		{
			std::vector<GridCell>::iterator cell = Cells.begin() + (size_t(y) * Size.x + x);
			cell->Position.x = x;
			cell->Position.y = y;
			cell->Solid = IsSolid(x, y, imageData);
			if (!cell->Solid)
			{
				if (x != 0 && IsSolid(x - 1, y, imageData))			// west side is closed, add a wall
					cell->CellTextures[Directions::XNeg] = walls;
 
 				if (x != Size.x -1 && IsSolid(x + 1, y, imageData))	// east side is closed, add a wall
 					cell->CellTextures[Directions::XPos] = walls;

				if (y != 0 && IsSolid(x, y - 1, imageData))			// north side is closed, add a wall
					cell->CellTextures[Directions::ZNeg] = walls;

				if (y != Size.y - 1 && IsSolid(x, y+1, imageData))	// east side is closed, add a wall
					cell->CellTextures[Directions::ZPos] = walls;

				cell->CellTextures[Directions::YNeg] = floor;
				cell->CellTextures[Directions::YPos] = ceiling;
			}
		}
	}
	FreeData(imageData);
}

const GridCell* GridMap::GetCell(int x, int y) const
{
	if (Cells.empty() || x < 0 || x >= Size.x || y < 0 || y >= Size.y)
		return nullptr;

	return &(*(Cells.begin() + (size_t(y) * size_t(Size.x) + x)));
}

void GridMap::DoForEachCell(std::function<void(GridCell* cell)> func)
{
	if (func == nullptr)
		return;

	for (GridCell& cell : Cells)
	{
		func(&cell);
	}
}
