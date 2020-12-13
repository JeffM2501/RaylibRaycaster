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
    Rectangle Bounds;

    bool checkedForHit;
    bool hitCell;
    bool currentCell;

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

class MapRenderer
{
public:
    MapRenderer();
    ~MapRenderer();

    void Setup(GridMap::Ptr map, float scale);
    void CleanUp();

    inline float GetDrawScale() { return DrawScale; }

    RenderCell* GetCell(int x, int y);
    RenderCell* GetCell(float x, float y);
    RenderCell* GetCell(const Vector3& cameraPos);
    RenderCell* GetCell(int index);
    RenderCell* GetDirectionCell(RenderCell* sourceCell, Directions dir);

    void DoForEachCell(std::function<void(RenderCell* cell)> func, bool visible = false);

    void ComputeVisibility(const Camera& camera, float fovX);
    void Draw();

    size_t SetupTexture(size_t textureID) const;

    bool CollideWithMap(Vector3& postion, float radius);

    bool DrawEverything = false;

    int DrawnCells = 0;
    int DrawnFaces = 0;

    typedef std::vector<RenderCell> RenderCellVec;
    typedef std::vector<RenderCell>::iterator RenderCellVecItr;
    RenderCellVec RenderCells;

    GridMap::Ptr MapPointer;

    std::vector<RaySet> DrawnRays;

    void BuildCellGeo(RenderCell* cell);

private:
    bool PointInCell(Vector2& postion, float radius, RenderCell* cellPtr);

    void CastRays(Vector2& origin);
    void GetTarget(RayCast::Ptr ray, Vector2& origin);

    void AddVisCell(RenderCell* cell);

    RenderFace MakeFace(Directions dir, CellParams* params, size_t material);

    void DrawCell(RenderCell* cell);
    void DrawFaces();

    float DrawScale = 1.0f;
    float RayAngleLimit = 1.0;

    std::map<Directions, Color> DirectionColors;

    std::deque<RaySet> PendingRayCasts;

    std::map<int, RenderCell*> VisibleCells;

    std::map<size_t, std::vector<RenderFace*>> FacesToDraw;
};
