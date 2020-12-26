#pragma once

#include "raylib.h"
#include "raylibExtras.h"
#include "raymath.h"
#include <cstdint>
#include <memory>
#include <map>
#include <vector>
#include <deque>

#include "Map.h"
#include "FaceGeometry.h"
#include "FPCamera.h"

class RenderFace
{
public:
    Mesh    FaceMesh;
    size_t  FaceMaterial;
};

class RenderCell
{
public:
    std::vector<RenderFace> RenderFaces;
    GridCell* MapCell;

    float GetFloorValue()
    {
        return MapCell->GetFloorValue();
    }

    float GetCeilingValue()
    {
        return MapCell->GetCeilingValue();
    }
};

class RayCast
{
public:
    Vector2 Ray;
    RenderCell* Target = nullptr;

    RayCast(): Ray(Vector2{0,0}){ }

    RayCast(Vector2 ray) : Ray(ray) { }

    typedef std::shared_ptr<RayCast> Ptr;
};

class RaySet
{
public:
	RayCast::Ptr Positive = nullptr;
	RayCast::Ptr Negative = nullptr;

    inline void Bisect(RaySet& posSide, RaySet& negSide)
    {
        RayCast::Ptr center = std::make_shared<RayCast>(Vector2Normalize(Vector2Add(Positive->Ray, Negative->Ray)));

        posSide.Positive = Positive;
        posSide.Negative = center;

        negSide.Positive = center;
        negSide.Negative = Negative;
    }
};

class MapVisibilitySet
{
public:
    FPCamera& ViewCamera;
	std::deque<RaySet> PendingRayCasts;
	std::map<int, RenderCell*> VisibleCells;
    std::map<int, RenderCell*> TargetCells;
	std::map<size_t, std::vector<RenderFace*>> FacesToDraw;
    
    std::vector<RaySet> DrawnRays;

    bool TrackDrwnRays = false;

	bool DrawEverything = false;

	int DrawnCells = 0;
	int DrawnFaces = 0;

    MapVisibilitySet(FPCamera& camera) : ViewCamera(camera) {}
};

class MapRenderer
{
public:
    MapRenderer();
    ~MapRenderer();

    void Setup(GridMap::Ptr map, float scale);
    void CleanUp();

    Vector2 ToMapPos(Vector3& postion);

    inline float GetDrawScale() { return DrawScale; }

    RenderCell* GetCell(int x, int y);
    RenderCell* GetCell(float x, float y);
    RenderCell* GetCell(const Vector3& cameraPos);
    RenderCell* GetCell(int index);
    RenderCell* GetDirectionCell(RenderCell* sourceCell, Directions dir);

    void DoForEachCell(std::function<void(RenderCell* cell)> func);
    void DoForEachVisibleCell(std::function<void(RenderCell* cell)> func, MapVisibilitySet& viewSet);

    void ComputeVisibility(MapVisibilitySet& viewSet);
    void Draw(MapVisibilitySet& viewSet);
    
	void BuildCellGeo(RenderCell* cell);
    size_t SetupTexture(size_t textureID) const;

    BoundingBox GetCellBBox(int index);

    typedef std::vector<RenderCell> RenderCellVec;
    typedef std::vector<RenderCell>::iterator RenderCellVecItr;
    RenderCellVec RenderCells;

    GridMap::Ptr MapPointer;

private:
    void CastRays(Vector2& origin, MapVisibilitySet& viewSet);
    void GetTarget(RayCast::Ptr ray, Vector2& origin, MapVisibilitySet& viewSet);

    void AddVisCell(RenderCell* cell, MapVisibilitySet& viewSet);

    RenderFace MakeFace(Directions dir, CellParams* params, FaceInfo& faceInfo);

    void DrawCell(RenderCell* cell, MapVisibilitySet& viewSet);
    void DrawFaces(MapVisibilitySet& viewSet);

    float DrawScale = 1.0f;
    float RayAngleLimit = 1.0;

    std::map<Directions, Color> DirectionColors;
};
