#include "MapRenderer.h"
#include "ResourceManager.h"
#include "FaceGeometry.h"
#include "MaterialManager.h"
#include "rlgl.h"

MapRenderer::MapRenderer()
{

}

MapRenderer::~MapRenderer()
{

}

size_t MapRenderer::SetupTexture(size_t textureID) const
{
    auto& texture = ResourceManager::GetTexture(textureID);
    SetTextureWrap(texture, WRAP_REPEAT);
    SetTextureFilter(texture, FILTER_ANISOTROPIC_16X);
    GenTextureMipmaps((Texture2D*)(&texture));
    return textureID;
}

BoundingBox MapRenderer::GetCellBBox(int index)
{
    BoundingBox bbox = { 0 };

    RenderCell* cell = GetCell(index);
    if (cell != nullptr)
    {
        bbox.min = Vector3{ (float)cell->MapCell->Position.x, cell->GetFloorValue(),(float)cell->MapCell->Position.y };
        bbox.max = Vector3{ (float)cell->MapCell->Position.x+1, cell->GetCeilingValue(),(float)cell->MapCell->Position.y+1 };
    }
    return bbox;
}

void MapRenderer::CleanUp()
{
    for (auto& mesh : RenderCells)
    {
        for (auto& f : mesh.RenderFaces)
            UnloadMesh(f.FaceMesh);
    }
    RenderCells.clear();
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

MeshGenerateCallback CallbackForDir(Directions dir)
{
    switch (dir)
    {
    case Directions::ZNeg:
        return GenNorthMesh;
    case Directions::ZPos:
        return GenSouthMesh;
    case Directions::XPos:
        return GenEastMesh;
    case Directions::XNeg:
        return GenWestMesh;
    case Directions::YNeg:
        return GenFloorMesh;
    case Directions::YPos:
        return GenCeilingMesh;
    }

    return nullptr;
}

RenderCell* MapRenderer::GetDirectionCell(RenderCell* sourceCell, Directions dir)
{
    if (sourceCell == nullptr)
        return nullptr;

    int xOffset = dir == Directions::XPos ? 1 : dir == Directions::XNeg ? -1 : 0;
    int yOffset = dir == Directions::ZPos ? 1 : dir == Directions::ZNeg ? -1 : 0;

    return GetCell(sourceCell->MapCell->Position.x + xOffset, sourceCell->MapCell->Position.y + yOffset);
}

RenderFace MapRenderer::MakeFace(Directions dir, CellParams* params, size_t material)
{
    RenderFace face;

	face.FaceMesh = GenMeshCustom(CallbackForDir(dir), params);
	face.FaceMaterial = MaterialManager::LoadTextureMaterial(MapPointer->MaterialList[material], DirectionColors[dir]);

    return face;
}

void MapRenderer::BuildCellGeo(RenderCell* cell)
{
    if (cell == nullptr)
        return;

    for (auto f : cell->RenderFaces)
        UnloadMesh(f.FaceMesh);

    cell->RenderFaces.clear();
    if (cell->MapCell->IsSolid())
        return;

	CellParams params;
	params.mapX = cell->MapCell->Position.x * DrawScale;
	params.mapY = cell->MapCell->Position.y * DrawScale;
    float floor = cell->GetFloorValue();
    float celing = cell->GetCeilingValue();

	for (auto& cellFace : cell->MapCell->CellTextures)
	{
		Directions dir = cellFace.first;

        bool fullFace = false;
        
		params.bottom = floor;
		params.top = celing;

        if (dir == Directions::YNeg || dir == Directions::YPos)
        {
			cell->RenderFaces.emplace_back(MakeFace(dir,&params,cellFace.second));
        }
        else
        {
            RenderCell* otherCell = GetDirectionCell(cell, dir);
            if (otherCell == nullptr)
                continue;

            float otherFloor = otherCell->GetFloorValue();
            float otherCeiling = otherCell->GetCeilingValue();

            if (floor >= otherFloor && celing <= otherCeiling) // no need for walls
                continue;

            // see if the other cell gives us a full face?
            if (otherCell->MapCell->IsSolid() || otherFloor > celing || otherCeiling < floor)
            {
                cell->RenderFaces.emplace_back(MakeFace(dir, &params, cellFace.second));
            }
            else
            {
				// see if the other cell has a higher floor than us

                if (otherFloor > floor)
                {
					params.bottom = floor;
					params.top = otherFloor;

                    cell->RenderFaces.emplace_back(MakeFace(dir, &params, cellFace.second));
                }

			    // see if the other cell has a lower ceiling than us

                if (otherCeiling < celing)
                {
                    params.bottom = otherCeiling;
					params.top = celing;

					cell->RenderFaces.emplace_back(MakeFace(dir, &params, cellFace.second));
                }
            }
        }
	}
}

void MapRenderer::Setup(GridMap::Ptr map, float scale)
{
    DrawScale = scale;
    MapPointer = map;

    MapPointer->MaterialAdded = GridMap::MaterialFunction([this](size_t id, const std::string& path)
        {
            SetupTexture(ResourceManager::GetAssetID(path));
        });

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

    for (auto& tx : MapPointer->MaterialList)
    {
        SetupTexture(ResourceManager::GetAssetID(tx.second));
    }

    RenderCells.clear();
    RenderCells.resize(map->GetCellCount());

    int count = 0;
    map->DoForEachCell([&count, this](GridCell* cell)
        {
            RenderCellVecItr renderCell = RenderCells.begin() + count;

            RenderCell& rCell = *renderCell;
            ++count;

            renderCell->MapCell = cell;
        });

	DoForEachCell([&count, this](RenderCell* cell)
		{
			BuildCellGeo(cell);
		});
}

void MapRenderer::DoForEachCell(std::function<void(RenderCell* cell)> func)
{
    if (func == nullptr)
        return;

	for (RenderCell& cell : RenderCells)
		func(&cell);
}

void MapRenderer::DoForEachVisibleCell(std::function<void(RenderCell* cell)> func, MapVisibilitySet& viewSet)
{
	if (func == nullptr)
		return;

	for (auto& cell : viewSet.VisibleCells)
		func(cell.second);
}

RenderCell* MapRenderer::GetCell(int x, int y)
{
    if (MapPointer == nullptr || RenderCells.empty() || x < 0 || x >= MapPointer->GetSize().x || y < 0 || y >= MapPointer->GetSize().y)
        return nullptr;

    return &(*(RenderCells.begin() + (size_t(y) * size_t(MapPointer->GetSize().x) + x)));
}

RenderCell* MapRenderer::GetCell(int index)
{
	if (MapPointer == nullptr || RenderCells.empty() || index < 0 || index >= RenderCells.size())
		return nullptr;

	return &(*(RenderCells.begin() + index));
}

RenderCell* MapRenderer::GetCell(float x, float y)
{
    return GetCell((int)std::floor(x), (int)std::floor(y));
}

RenderCell* MapRenderer::GetCell(const Vector3& cameraPos)
{
    return GetCell(cameraPos.x / DrawScale, cameraPos.z / DrawScale);
}


Vector2 MapRenderer::ToMapPos(Vector3& postion)
{
    return Vector2{ postion.x / DrawScale, postion.z / DrawScale };
}

void MapRenderer::AddVisCell(RenderCell* cell, MapVisibilitySet& viewSet)
{
    if (cell == nullptr)
        return;

    if (cell->MapCell->IsSolid())
    {
		if (viewSet.TargetCells.find(cell->MapCell->Index) == viewSet.TargetCells.end())
			viewSet.TargetCells.emplace(cell->MapCell->Index, cell);
    }
    else
    {
        if (viewSet.VisibleCells.find(cell->MapCell->Index) == viewSet.VisibleCells.end())
            viewSet.VisibleCells.emplace(cell->MapCell->Index, cell);
    }
}

void MapRenderer::GetTarget(RayCast::Ptr ray, Vector2& origin, MapVisibilitySet& viewSet)
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
    AddVisCell(walkcell, viewSet);

    while (walkcell != nullptr && !walkcell->MapCell->IsSolid())
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

        if (walkcell != nullptr && !walkcell->MapCell->IsSolid())
            AddVisCell(walkcell, viewSet);

        if (walkcell != nullptr)
            ray->Target = walkcell;
    }
}

void MapRenderer::CastRays(Vector2& origin, MapVisibilitySet& viewSet)
{
    while (!viewSet. PendingRayCasts.empty())
    {
        RaySet set = viewSet.PendingRayCasts.front();
        viewSet.PendingRayCasts.pop_front();

        if (viewSet.TrackDrwnRays)
            viewSet.DrawnRays.emplace_back(set);

        //if our rays aren't cast then cast them and get our targets
        if (set.Positive->Target == nullptr)
            GetTarget(set.Positive, origin, viewSet);

        if (set.Negative->Target == nullptr)
            GetTarget(set.Negative, origin, viewSet);

        // both are hitting the same target, we are done
        if (set.Positive->Target != nullptr && set.Negative->Target != nullptr)
        {
            if (set.Positive->Target->MapCell->Index != set.Negative->Target->MapCell->Index) // split and continue
            {
                if (Vector2DotProduct(set.Positive->Ray, set.Negative->Ray) < RayAngleLimit)
                {
                    RaySet posSet;
                    RaySet negSeg;
                    set.Bisect(posSet, negSeg);

                    viewSet.PendingRayCasts.push_back(posSet);
                    viewSet.PendingRayCasts.push_back(negSeg);
                }
            }
        }
    }
}

void MapRenderer::ComputeVisibility(MapVisibilitySet& viewSet)
{
    viewSet.VisibleCells.clear();
    viewSet.TargetCells.clear();

    const Camera& camera = viewSet.ViewCamera.GetCamera();

    Vector2 mapCamera{ camera.position.x / DrawScale , camera.position.z / DrawScale };
    Vector2 viewVector{ camera.target.x - camera.position.x,camera.target.z - camera.position.z };
    viewVector = Vector2Normalize(viewVector);

    // our cell and those around us are always visible
    AddVisCell(GetCell(mapCamera.x, mapCamera.y), viewSet);
    AddVisCell(GetCell(mapCamera.x - 1, mapCamera.y), viewSet);
    AddVisCell(GetCell(mapCamera.x + 1, mapCamera.y), viewSet);
    AddVisCell(GetCell(mapCamera.x, mapCamera.y - 1), viewSet);
    AddVisCell(GetCell(mapCamera.x - 1, mapCamera.y - 1), viewSet);
    AddVisCell(GetCell(mapCamera.x + 1, mapCamera.y - 1), viewSet);
    AddVisCell(GetCell(mapCamera.x, mapCamera.y + 1), viewSet);
    AddVisCell(GetCell(mapCamera.x - 1, mapCamera.y + 1), viewSet);
    AddVisCell(GetCell(mapCamera.x + 1, mapCamera.y + 1), viewSet);

    Vector2 posFOV = Vector2Rotate(viewVector, viewSet.ViewCamera.GetFOVX() * 0.5f);
    Vector2 negFov = Vector2Rotate(viewVector, -viewSet.ViewCamera.GetFOVX() * 0.5f);

    // recursively raycast between vectors until we are done.
    RaySet viewRays;
    viewRays.Positive = std::make_shared<RayCast>(posFOV);
    viewRays.Negative = std::make_shared<RayCast>(negFov);

    viewSet.DrawnRays.clear();
    viewSet.PendingRayCasts.clear();
    viewSet.PendingRayCasts.push_back(viewRays);
    CastRays(mapCamera,viewSet);
}

void MapRenderer::DrawFaces(MapVisibilitySet& viewSet)
{
    Vector3 pos{ 0, 0, 0 };

    Matrix transform = MatrixIdentity();

    for (auto& faceListItr : viewSet.FacesToDraw)
    {
        const Material& material = MaterialManager::GetRuntimeMaterial(faceListItr.first);
        for (auto& face : faceListItr.second)
        {
            rlDrawMesh(face->FaceMesh, material, transform);
        }
    }
}

void MapRenderer::DrawCell(RenderCell* cell, MapVisibilitySet& viewSet)
{
    ++viewSet.DrawnCells;
    for (auto& face : cell->RenderFaces)
    {
        ++viewSet.DrawnFaces;
        viewSet.FacesToDraw[face.FaceMaterial].push_back(&face);
    }
}

void MapRenderer::Draw(MapVisibilitySet& viewSet)
{
    viewSet.FacesToDraw.clear();
    viewSet.DrawnCells = 0;
    viewSet.DrawnFaces = 0;

    if (viewSet.DrawEverything)
    {
        for (auto& cell : RenderCells)
            DrawCell(&cell, viewSet);
    }
    else
    {
        for (auto vis : viewSet.VisibleCells)
            DrawCell(vis.second, viewSet);
    }

    DrawFaces(viewSet);
}