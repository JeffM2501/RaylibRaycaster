#pragma once

#include "raylib.h"
#include "raylibExtras.h"
#include <cstdint>
#include <memory>
#include <map>
#include <vector>


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
	Vector2 Position;
	bool Solid;
};

class CastMap 
{
public:
    CastMap() {}
	virtual ~CastMap();
	void LoadFromImage(const Image& image, float scale, size_t walls, size_t floor, size_t ceiling);

	const GridCell* GetCell(int x, int y);

	int Width = 0;
	int Height = 0;

	std::vector<GridCell> Cells;

	typedef std::shared_ptr<CastMap> Ptr;
};

class RenderCell
{
public:
	std::map<Directions,uint16_t> RenderFaces;
	GridCell* MapCell;
	Rectangle Bounds;

	bool checkedForHit;
	bool hitCell;
	bool currentCell;
};

class MapRenderer
{
public:
	typedef std::vector<RenderCell> RenderCellVec;
	typedef std::vector<RenderCell>::iterator RenderCellVecItr;
	RenderCellVec RenderCells;

	CastMap::Ptr MapPointer;

	MapRenderer();
	~MapRenderer();

	void Setup(CastMap::Ptr map, float scale);
	void CleanUp();

	RenderCell* GetCell(int x, int y);

	void Draw();
	void DrawMiniMap(int posX, int posY, int scale, Camera& camera, float fovX);
	void DrawMiniMapZoomed(int posX, int posY, int scale, Camera& camera, float fovX);

	size_t SetupTexture(size_t textureID);

	bool PointIsOpen(Vector3& postion, float radius);

private:
	float DrawScale = 1.0f;

	std::map<Directions, Mesh> DirectionMeshes;
	std::map<uint16_t, Model> ModelCache;

	std::map<Directions, Color> DirectionColors;

	uint16_t GetModelFromCache(size_t hash, std::map<size_t, uint16_t>& cache, Mesh& mesh);

	bool PointInCell(Vector2& postion, float radius, RenderCell* cellPtr);
};

