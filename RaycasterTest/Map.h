#pragma once

#include "raylib.h"
#include "raylibExtras.h"
#include "raymath.h"
#include <cstdint>
#include <memory>
#include <functional>
#include <map>
#include <vector>
#include <deque>

enum class Directions
{
	ZNeg,	// north
	ZPos,	// south
	XPos,	// east
	XNeg,	// west
	YNeg,	// floor
	YPos,	// ceiling
};

class GridCell
{
public:
	std::map<Directions, size_t>  CellTextures;
	Vector2i Position = { 0,0 };
	bool Solid = false;
};

class GridMap 
{
public:
    GridMap() {}
	virtual ~GridMap();
	void LoadFromImage(const Image& image, float scale, size_t walls, size_t floor, size_t ceiling);

	const GridCell* GetCell(int x, int y) const;
	inline const Vector2i GetSize() const { return Size; }
	inline size_t GetCellCount() const { return Cells.size(); };

	void DoForEachCell(std::function<void(GridCell* cell)> func);

	typedef std::shared_ptr<GridMap> Ptr;

private:
	Vector2i Size = { 0, 0 };
    std::vector<GridCell> Cells;

	bool IsSolid(int x, int y, Color* imageData);
};

