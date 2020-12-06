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
	ZNeg = 0,	// north
	ZPos = 1,	// south
	XPos = 2,	// east
	XNeg = 3,	// west
	YNeg = 4,	// floor
	YPos = 5,	// ceiling
};

class GridCell
{
public:
	std::map<Directions, size_t>  CellTextures;
	uint8_t Floor = 128;
	uint8_t Ceiling = 16;
	Vector2i Position = { 0,0 };
	inline bool IsSolid(){ return Floor == 255; }
};

class GridMap 
{
public:
    GridMap() {}
	virtual ~GridMap();
	void LoadFromImage(const Image& image, float scale, size_t walls, size_t floor, size_t ceiling);

	bool LoadFromFile(const std::string& path);
	void SaveToFile(const std::string& path);

	const GridCell* GetCell(int x, int y) const;
	inline const Vector2i GetSize() const { return Size; }
	inline size_t GetCellCount() const { return Cells.size(); };

	void DoForEachCell(std::function<void(GridCell* cell)> func);

	size_t AddMaterial(const std::string& path);

	std::map<size_t, std::string> MaterialList;

	typedef std::shared_ptr<GridMap> Ptr;

private:
	Vector2i Size = { 0, 0 };
    std::vector<GridCell> Cells;
};

