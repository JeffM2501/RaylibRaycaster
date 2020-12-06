#include "MapRenderer.h"
#include "ResourceManager.h"
#include "FaceGeometry.h"

MapRenderer::MapRenderer()
{

}

MapRenderer::~MapRenderer()
{

}

size_t MapRenderer::SetupTexture(size_t textureID)
{
    auto& texture = ResourceManager::GetTexture(textureID);
    SetTextureWrap(texture, WRAP_CLAMP);
    SetTextureFilter(texture, FILTER_ANISOTROPIC_16X);
    GenTextureMipmaps((Texture2D*)(&texture));
    return textureID;
}

void MapRenderer::CleanUp()
{
    RenderCells.clear();
    for (auto& cacheItr : ModelCache)
        UnloadModelNoMesh(cacheItr.second);

    for (auto& meshItr : DirectionMeshes)
        UnloadMesh(meshItr.second);
}

uint16_t MapRenderer::GetModelFromCache(size_t hash, std::map<size_t, uint16_t>& cache, Mesh& mesh)
{
    uint16_t id = uint16_t(ModelCache.size() + 1);

    auto itr = cache.find(hash);
    if (itr == cache.end())
    {
        uint16_t id = uint16_t(ModelCache.size() + 1);
        Model model = LoadModelFromMesh(mesh);

        if (hash != 0)
            SetMaterialTexture(model.materials, MAP_DIFFUSE, ResourceManager::GetTexture(hash));

        cache[hash] = id;

        ModelCache.emplace(id, model);
    }
    else
        id = itr->second;

    return id;
}

Color ColorFromNormal(Vector3 normal, Vector3& lightVector, float ambient, float boost = 1.0f)
{
    float param = 0;

    float dot = Vector3DotProduct(normal, lightVector);
    if (dot < 0)
        param = dot * -1;

    float factor = Lerp(ambient, 1, param);
    factor *= boost;
    factor = std::min(factor, 1.0f);

    return ColorFromNormalized(Vector4{ factor, factor, factor,1 });
}

void MapRenderer::Setup(GridMap::Ptr map, float scale)
{
    if (DirectionMeshes.size() == 0)
    {
        DirectionMeshes[Directions::XNeg] = GenMeshCustom(&GenWestMesh, nullptr);
        DirectionMeshes[Directions::XPos] = GenMeshCustom(&GenEastMesh, nullptr);

        DirectionMeshes[Directions::ZNeg] = GenMeshCustom(&GenNorthMesh, nullptr);
        DirectionMeshes[Directions::ZPos] = GenMeshCustom(&GenSouthMesh, nullptr);

        DirectionMeshes[Directions::YPos] = GenMeshCustom(&GenCeilingMesh, nullptr);
        DirectionMeshes[Directions::YNeg] = GenMeshCustom(&GenFloorMesh, nullptr);
    }

    MaterialIndexMap.clear();
    for (auto& mat : map->MaterialList)
    {
        size_t materialID = SetupTexture(ResourceManager::GetAssetID(mat.second));
        MaterialIndexMap[mat.first] = materialID;
    }

    DrawScale = scale;
    MapPointer = map;

    float maxMapSize = Vector2Length(Vector2{ float(map->GetSize().x), float(map->GetSize().y) });

    Vector2 v1 = { 1,0 };
    Vector2 v2 = Vector2Normalize(Vector2{ maxMapSize, 0.025f });

    RayAngleLimit = Vector2DotProduct(v1, v2);

    Vector3 sundir = Vector3Normalize(Vector3{ 1,-1, 0.5f });
    float ambient = 0.25f;

    // compute the lighting colors for each direction
    DirectionColors[Directions::YNeg] = ColorFromNormal(Vector3{ 0,1,0 }, sundir, ambient, 5);
    DirectionColors[Directions::YPos] = ColorFromNormal(Vector3{ 0,-1,0 }, sundir, ambient, 4);

    DirectionColors[Directions::XNeg] = ColorFromNormal(Vector3{ 1,0,0 }, sundir, ambient);
    DirectionColors[Directions::XPos] = ColorFromNormal(Vector3{ -1,0,0 }, sundir, ambient);

    DirectionColors[Directions::ZNeg] = ColorFromNormal(Vector3{ 0,0,1 }, sundir, ambient);
    DirectionColors[Directions::ZPos] = ColorFromNormal(Vector3{ 0,0,-1 }, sundir, ambient);

    RenderCells.clear();
    RenderCells.resize(map->GetCellCount());

    std::map<Directions, std::map<size_t, uint16_t>> caches;

    int count = 0;
    map->DoForEachCell([&count, &caches,this](GridCell* cell)
        {
            RenderCellVecItr renderCell = RenderCells.begin() + count;
            renderCell->Index = count;

            ++count;

            renderCell->MapCell = cell;
            renderCell->Bounds = Rectangle{ cell->Position.x * DrawScale, cell->Position.y * DrawScale, DrawScale, DrawScale };

            for (auto& cellFace : cell->CellTextures)
            {
                Directions dir = cellFace.first;
                size_t materialID = MaterialIndexMap[cellFace.second];

                if (caches.find(dir) == caches.end())
                    caches[dir] = std::map<size_t, uint16_t>();

                renderCell->RenderFaces[dir] = GetModelFromCache(materialID, caches[dir], DirectionMeshes[dir]);
            }
        });
}


void MapRenderer::DoForEachCell(std::function<void(RenderCell* cell)> func, bool visible)
{
    if (func == nullptr)
        return;

    if (visible)
    {
        for (auto& cell : VisibleCells)
            func(cell.second);
    }
    else
    {
        for (RenderCell& cell : RenderCells)
            func(&cell);
    }
}

RenderCell* MapRenderer::GetCell(int x, int y)
{
    if (MapPointer == nullptr || RenderCells.empty() || x < 0 || x >= MapPointer->GetSize().x || y < 0 || y >= MapPointer->GetSize().y)
        return nullptr;

    return &(*(RenderCells.begin() + (size_t(y) * size_t(MapPointer->GetSize().x) + x)));
}

RenderCell* MapRenderer::GetCell(float x, float y)
{
    return GetCell((int)std::floor(x), (int)std::floor(y));
}

bool MapRenderer::PointInCell(Vector2& postion, float radius, RenderCell* cellPtr)
{
    if (cellPtr == nullptr || cellPtr->MapCell == nullptr)
        return true;

    cellPtr->checkedForHit = true;

    if (!cellPtr->MapCell->Solid)
        return false;

    radius *= DrawScale;

    float minX = cellPtr->Bounds.x - radius;
    float maxX = cellPtr->Bounds.x + cellPtr->Bounds.width + radius;
    float minY = cellPtr->Bounds.y - radius;
    float maxY = cellPtr->Bounds.y + cellPtr->Bounds.height + radius;

    if (postion.x < minX || postion.x > maxX)
        return false;

    if (postion.y < minY || postion.y > maxY)
        return false;

    cellPtr->hitCell = true;
    return true;
}

bool MapRenderer::CollideWithMap(Vector3& postion, float radius)
{
    for (auto& cell : RenderCells)
    {
        cell.checkedForHit = false;
        cell.hitCell = false;
        cell.currentCell = false;
    }

    Vector2 flatPos{ postion.x, postion.z };

    int mapX = (int)std::floor(flatPos.x / DrawScale);
    int mapY = (int)std::floor(flatPos.y / DrawScale);

    auto ptr = GetCell(mapX, mapY);
    ptr->currentCell = true;

    if (PointInCell(flatPos, radius, ptr))
        return true;
    if (PointInCell(flatPos, radius, GetCell(mapX + 1, mapY)))
        return true;
    if (PointInCell(flatPos, radius, GetCell(mapX - 1, mapY)))
        return true;

    if (PointInCell(flatPos, radius, GetCell(mapX, mapY + 1)))
        return true;
    if (PointInCell(flatPos, radius, GetCell(mapX + 1, mapY + 1)))
        return true;
    if (PointInCell(flatPos, radius, GetCell(mapX - 1, mapY + 1)))
        return true;

    if (PointInCell(flatPos, radius, GetCell(mapX, mapY - 1)))
        return true;
    if (PointInCell(flatPos, radius, GetCell(mapX + 1, mapY - 1)))
        return true;
    if (PointInCell(flatPos, radius, GetCell(mapX - 1, mapY - 1)))
        return true;

    return false;
}

void MapRenderer::AddVisCell(RenderCell* cell)
{
    if (cell == nullptr)
        return;

    if (VisibleCells.find(cell->Index) == VisibleCells.end())
        VisibleCells.emplace(cell->Index, cell);
}

void MapRenderer::GetTarget(RayCast::Ptr ray, Vector2& origin)
{
    if (ray->Target != nullptr)
        return;

    int mapX = (int)std::floor(origin.x);
    int mapY = (int)std::floor(origin.y);

    double sideDistX = 0;
    double sideDistY = 0;

    double deltaDistX = std::abs(1.0 / ray->Ray.x);
    double deltaDistY = std::abs(1.0 / ray->Ray.y);

    int stepX = 0;
    int stepY = 0;
    int hit = 0;

    if (ray->Ray.x < 0)
    {
        stepX = -1;
        sideDistX = (origin.x - mapX) * deltaDistX;
    }
    else
    {
        stepX = 1;
        sideDistX = (mapX + 1.0 - origin.x) * deltaDistX;
    }
    if (ray->Ray.y < 0)
    {
        stepY = -1;
        sideDistY = (origin.y - mapY) * deltaDistY;
    }
    else
    {
        stepY = 1;
        sideDistY = (mapY + 1.0 - origin.y) * deltaDistY;
    }

    RenderCell* walkcell = GetCell(mapX, mapY);
    AddVisCell(walkcell);

    while (walkcell != nullptr && !walkcell->MapCell->Solid)
    {
        //jump to next map square, OR in x-direction, OR in y-direction
        if (sideDistX < sideDistY)
        {
            sideDistX += deltaDistX;
            mapX += stepX;
        }
        else
        {
            sideDistY += deltaDistY;
            mapY += stepY;

        }
        walkcell = GetCell(mapX, mapY);

        if (walkcell != nullptr && !walkcell->MapCell->Solid)
            AddVisCell(walkcell);

        if (walkcell != nullptr)
            ray->Target = walkcell;
    }
}

void MapRenderer::CastRays(Vector2& origin)
{
    while (!PendingRayCasts.empty())
    {
        RaySet set = PendingRayCasts.front();
        PendingRayCasts.pop_front();

        DrawnRays.emplace_back(set);

        //if our rays aren't cast then cast them and get our targets
        if (set.Positive->Target == nullptr)
            GetTarget(set.Positive, origin);

        if (set.Negative->Target == nullptr)
            GetTarget(set.Negative, origin);

        // both are hitting the same target, we are done
        if (set.Positive->Target != nullptr && set.Negative->Target != nullptr)
        {
            if (set.Positive->Target->Index != set.Negative->Target->Index) // split and continue
            {
                if (Vector2DotProduct(set.Positive->Ray, set.Negative->Ray) < RayAngleLimit)
                {
                    RaySet posSet;
                    RaySet negSeg;
                    set.Bisect(posSet, negSeg);

                    PendingRayCasts.push_back(posSet);
                    PendingRayCasts.push_back(negSeg);
                }
            }
        }
    }
}

void MapRenderer::ComputeVisibility(const Camera& camera, float fovX)
{
    VisibleCells.clear();

    Vector2 mapCamera{ camera.position.x / DrawScale , camera.position.z / DrawScale };
    Vector2 viewVector{ camera.target.x - camera.position.x,camera.target.z - camera.position.z };
    viewVector = Vector2Normalize(viewVector);

    // our cell and those around us are always visible
    AddVisCell(GetCell(mapCamera.x, mapCamera.y));
    AddVisCell(GetCell(mapCamera.x - 1, mapCamera.y));
    AddVisCell(GetCell(mapCamera.x + 1, mapCamera.y));
    AddVisCell(GetCell(mapCamera.x, mapCamera.y - 1));
    AddVisCell(GetCell(mapCamera.x - 1, mapCamera.y - 1));
    AddVisCell(GetCell(mapCamera.x + 1, mapCamera.y - 1));
    AddVisCell(GetCell(mapCamera.x, mapCamera.y + 1));
    AddVisCell(GetCell(mapCamera.x - 1, mapCamera.y + 1));
    AddVisCell(GetCell(mapCamera.x + 1, mapCamera.y + 1));

    Vector2 posFOV = Vector2Rotate(viewVector, fovX * 0.5f);
    Vector2 negFov = Vector2Rotate(viewVector, -fovX * 0.5f);

    // recursively raycast between vectors until we are done.
    RaySet viewSet;
    viewSet.Positive = std::make_shared<RayCast>(posFOV);
    viewSet.Negative = std::make_shared<RayCast>(negFov);

    DrawnRays.clear();
    PendingRayCasts.clear();
    PendingRayCasts.push_back(viewSet);
    CastRays(mapCamera);
}

void MapRenderer::DrawFaces()
{
    Vector3 pos{ 0, 0, 0 };

    for (auto& faceListItr : FacesToDraw)
    {
        for (auto& face : faceListItr.second)
        {
            RenderCell* cell = face.Cell;

            pos.x = cell->MapCell->Position.x * DrawScale;
            pos.z = cell->MapCell->Position.y * DrawScale;

            if (face.Dir == Directions::YPos)
                pos.y = DrawScale;
            else
                pos.y = 0;

            DrawModel(ModelCache[faceListItr.first], pos, DrawScale, DirectionColors[face.Dir]);
        }
    }
}

void MapRenderer::DrawCell(RenderCell* cell)
{
    ++DrawnCells;
    for (auto& face : cell->RenderFaces)
    {
        ++DrawnFaces;
        FacesToDraw[face.second].push_back(FaceDraw{ cell, face.first });
    }
}

void MapRenderer::Draw()
{
    FacesToDraw.clear();
    DrawnCells = 0;
    DrawnFaces = 0;

    if (DrawEverything)
    {
        for (auto& cell : RenderCells)
            DrawCell(&cell);
    }
    else
    {
        for (auto vis : VisibleCells)
            DrawCell(vis.second);
    }

    DrawFaces();
}