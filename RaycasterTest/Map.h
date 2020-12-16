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

constexpr float DepthIncrement = 0.0625f;

class GridCell
{
public:
	int Index = 0;
	std::map<Directions, size_t>  CellTextures;
	uint8_t Floor = 128;
	uint8_t Ceiling = 16;
	Vector2i Position = { 0,0 };
	inline bool IsSolid() { return Floor == 255; }

	float GetFloorValue()
	{
		return Floor * DepthIncrement;
	}

	float GetCeilingValue()
	{
		return GetFloorValue() + Ceiling * DepthIncrement;
	}
};

class GridMap 
{
public:
    GridMap() {}
	virtual ~GridMap();
	void LoadFromImage(const Image& image, float scale, size_t walls, size_t floor, size_t ceiling);

	bool LoadFromFile(const std::string& path);
	void SaveToFile(const std::string& path);

	GridCell* GetCell(int x, int y);
	GridCell* GetCell(int index);
	GridCell* GetDirectionCell(GridCell* sourceCell, Directions dir);
	GridCell* GetDirectionCell(int sourceIndex, Directions dir);

	inline const Vector2i GetSize() const { return Size; }
	inline size_t GetCellCount() const { return Cells.size(); };

	void DoForEachCell(std::function<void(GridCell* cell)> func);

	bool PointInCell(Vector2& postion, float radius, GridCell* cell);
	bool CollideWithMap(Vector2&& postion, float radius);

	typedef std::function<void(size_t id, const std::string& path)> MaterialFunction;
	MaterialFunction MaterialAdded;

	size_t AddMaterial(const std::string& path);

	std::map<size_t, std::string> MaterialList;

	typedef std::shared_ptr<GridMap> Ptr;

private:
	Vector2i Size = { 0, 0 };
    std::vector<GridCell> Cells;
};

