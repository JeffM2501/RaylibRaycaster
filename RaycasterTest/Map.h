#pragma once

#include "raylib.h"
#include "raylibExtras.h"
#include "raymath.h"
#include <cstdint>
#include <memory>
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
	int Index = 0;
	std::map<Directions,uint16_t> RenderFaces;
	GridCell* MapCell;
	Rectangle Bounds;

	bool checkedForHit;
	bool hitCell;
	bool currentCell;
};

class RayCast
{
public:
	Vector2 Ray;
	RenderCell* target;

	RayCast() {}
	RayCast( Vector2 ray ) : Ray(ray){ }

	typedef std::shared_ptr<RayCast> Ptr;
};

class RaySet
{
public:
	RayCast::Ptr Positive;
	RayCast::Ptr Negative;


	inline void Bisect(RaySet& posSide, RaySet& negSide)
	{
		RayCast::Ptr center = std::make_shared<RayCast>(Vector2Normalize(Vector2Add(Positive->Ray, Negative->Ray)));

		posSide.Positive = Positive;
		posSide.Negative = center;

		negSide.Positive = center;
		negSide.Negative = Negative;
	}
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
	RenderCell* GetCell(float x, float y);

	void ComputeVisibility(const Camera& camera, float fovX);

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

	void CastRays(Vector2& origin);
	void GetTarget(RayCast::Ptr ray, Vector2& origin);

	void AddVisCell(RenderCell* cell);

	std::deque< RaySet> PendingRayCasts;
	std::map<int,RenderCell*> VisibleCells;
};

