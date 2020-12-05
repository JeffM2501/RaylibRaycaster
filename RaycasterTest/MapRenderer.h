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

class RenderCell
{
public:
    int Index = 0;
    std::map<Directions, uint16_t> RenderFaces;
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
    RenderCell* Target = nullptr;

    RayCast() {}
    RayCast(Vector2 ray) : Ray(ray) { }

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
    MapRenderer();
    ~MapRenderer();

    void Setup(GridMap::Ptr map, float scale);
    void CleanUp();

    inline float GetDrawScale() { return DrawScale; }

    RenderCell* GetCell(int x, int y);
    RenderCell* GetCell(float x, float y);

    void DoForEachCell(std::function<void(RenderCell* cell)> func, bool visible = false);

    void ComputeVisibility(const Camera& camera, float fovX);
    void Draw();

    size_t SetupTexture(size_t textureID);

    bool CollideWithMap(Vector3& postion, float radius);

    bool DrawEverything = false;

    int DrawnCells = 0;
    int DrawnFaces = 0;

    typedef std::vector<RenderCell> RenderCellVec;
    typedef std::vector<RenderCell>::iterator RenderCellVecItr;
    RenderCellVec RenderCells;

    GridMap::Ptr MapPointer;

    std::vector<RaySet> DrawnRays;

private:
    uint16_t GetModelFromCache(size_t hash, std::map<size_t, uint16_t>& cache, Mesh& mesh);

    bool PointInCell(Vector2& postion, float radius, RenderCell* cellPtr);

    void CastRays(Vector2& origin);
    void GetTarget(RayCast::Ptr ray, Vector2& origin);

    void AddVisCell(RenderCell* cell);

    void DrawCell(RenderCell* cell);
    void DrawFaces();

    float DrawScale = 1.0f;
    float RayAngleLimit = 1.0;

    std::map<Directions, Mesh> DirectionMeshes;
    std::map<uint16_t, Model> ModelCache;

    std::map<Directions, Color> DirectionColors;

    std::deque<RaySet> PendingRayCasts;

    std::map<int, RenderCell*> VisibleCells;

    class FaceDraw
    {
    public:
        RenderCell* Cell;
        Directions Dir;
    };

    std::map<uint16_t, std::vector<FaceDraw>> FacesToDraw;
};
