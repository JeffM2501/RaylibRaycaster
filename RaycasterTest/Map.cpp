#include "Map.h"
#include "ResourceManager.h"
#include "raymath.h"

#include <algorithm>
#include <cmath>

CastMap::~CastMap()
{
}

bool IsSolid(int x, int y, Color* imageData, int width)
{
	return (imageData + (y * width + x))->b > 0;
}

void CastMap::LoadFromImage(const Image& image, float scale, size_t walls, size_t floor, size_t ceiling)
{
	Width = image.width;
	Height = image.height;

	Cells.clear();

	Cells.resize(size_t(Width) * size_t(Height));
	Color* imageData = GetImageData(image);

	for (int y = 0; y < Height; y++)
	{
		for (int x = 0; x < Width; x++)
		{
			std::vector<GridCell>::iterator cell = Cells.begin() + (size_t(y) * Width + x);
			cell->Position.x = (float)x;
			cell->Position.y = (float)y;
			cell->Solid = IsSolid(x, y, imageData, Width);
			if (!cell->Solid)
			{
				if (x != 0 && IsSolid(x - 1, y, imageData, Width))			// west side is closed, add a wall
					cell->CellTextures[Directions::XNeg] = walls;
 
 				if (x != Width-1 && IsSolid(x + 1, y, imageData, Width))	// east side is closed, add a wall
 					cell->CellTextures[Directions::XPos] = walls;

				if (y != 0 && IsSolid(x, y - 1, imageData, Width))			// north side is closed, add a wall
					cell->CellTextures[Directions::ZNeg] = walls;

				if (y != Height - 1 && IsSolid(x, y+1, imageData, Width))	// east side is closed, add a wall
					cell->CellTextures[Directions::ZPos] = walls;

				cell->CellTextures[Directions::YNeg] = floor;
				cell->CellTextures[Directions::YPos] = ceiling;
			}
		}
	}
	FreeData(imageData);
}

const GridCell* CastMap::GetCell(int x, int y)
{
	if (Cells.empty() || x < 0 || x >= Width || y < 0 || y >= Height)
		return nullptr;

	return &(*(Cells.begin() + (size_t(y) * size_t(Width) + x)));
}

constexpr float xmin = -0.001f;
constexpr float xmax = 1.001f;

constexpr float zmin = -0.001f;
constexpr float zmax = 1.001f;

constexpr float ymin = 0;
constexpr float ymax = 1;


void GenCeilingMesh(Mesh* mesh, void* userData)
{
	float width = 1;
	float height = 1;
	float length = 1;

	float vertices[] =
	{
		xmin,ymin,zmin,
		xmax,ymin,zmin,
		xmin,ymin,zmax,
		xmax,ymin,zmax,
	};

	float texcoords[] =
	{
		1,0,
		0,0,
		1,1,
		0,1,
	};

	float normals[] =
	{
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
	};

	AllocateMeshData(mesh, 4, 2, 0, true, false);
	memcpy(mesh->vertices, vertices, 4 * 3 * sizeof(float));
	memcpy(mesh->texcoords, texcoords, 4 * 2 * sizeof(float));
	memcpy(mesh->normals, normals, 4 * 3 * sizeof(float));

	mesh->indices[0] = 0;
	mesh->indices[1] = 1;
	mesh->indices[2] = 2;

	mesh->indices[3] = 1;
	mesh->indices[4] = 3;
	mesh->indices[5] = 2;

	mesh->vertexCount = 4;
	mesh->triangleCount = 2;
}

void GenFloorMesh(Mesh* mesh, void* userData)
{
	float width = 1;
	float height = 1;
	float length = 1;

	float vertices[] =
	{
		xmin,ymin,zmin,
		xmax,ymin,zmin,
		xmin,ymin,zmax,
		xmax,ymin,zmax,
	};

	float texcoords[] =
	{
		0,0,
		1,0,
		0,1,
		1,1

	};

	float normals[] = 
	{
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
	};

	AllocateMeshData(mesh, 4, 2, 0, true, false);
	memcpy(mesh->vertices, vertices, 4 * 3 * sizeof(float));
	memcpy(mesh->texcoords, texcoords, 4 * 2 * sizeof(float));
	memcpy(mesh->normals, normals, 4 * 3 * sizeof(float));

	mesh->indices[0] = 2;
	mesh->indices[1] = 1;
	mesh->indices[2] = 0;

	mesh->indices[3] = 2;
	mesh->indices[4] = 3;
	mesh->indices[5] = 1;

	mesh->vertexCount = 4;
	mesh->triangleCount = 2;
}

void GenWestMesh(Mesh* mesh, void* userData)
{
	float width = 1;
	float height = 1;
	float length = 1;

	float vertices[] =
	{
		xmin,ymax,zmin,
		xmin,ymin,zmin,
		xmin,ymax,zmax,
		xmin,ymin,zmax,
	};

	float texcoords[] =
	{
		1,0,
		1,1,
		0,0,
		0,1,
	};

	float normals[] =
	{
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
	};

	AllocateMeshData(mesh, 4, 2, 0, true, false);
	memcpy(mesh->vertices, vertices, 4 * 3 * sizeof(float));
	memcpy(mesh->texcoords, texcoords, 4 * 2 * sizeof(float));
	memcpy(mesh->normals, normals, 4 * 3 * sizeof(float));

	mesh->indices[0] = 2;
	mesh->indices[1] = 1;
	mesh->indices[2] = 0;

	mesh->indices[3] = 2;
	mesh->indices[4] = 3;
	mesh->indices[5] = 1;

	mesh->vertexCount = 4;
	mesh->triangleCount = 2;
}

void GenEastMesh(Mesh* mesh, void* userData)
{
	float width = 1;
	float height = 1;
	float length = 1;

	float vertices[] =
	{
		xmax,ymax,zmin,
		xmax,ymin,zmin,
		xmax,ymax,zmax,
		xmax,ymin,zmax,
	};

	float texcoords[] =
	{
		0,0,
		0,1,
		1,0,
		1,1,
	};

	float normals[] =
	{
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
	};

	AllocateMeshData(mesh, 4, 2, 0, true, false);
	memcpy(mesh->vertices, vertices, 4 * 3 * sizeof(float));
	memcpy(mesh->texcoords, texcoords, 4 * 2 * sizeof(float));
	memcpy(mesh->normals, normals, 4 * 3 * sizeof(float));

	mesh->indices[0] = 0;
	mesh->indices[1] = 1;
	mesh->indices[2] = 2;

	mesh->indices[3] = 1;
	mesh->indices[4] = 3;
	mesh->indices[5] = 2;

	mesh->vertexCount = 4;
	mesh->triangleCount = 2;
}

void GenSouthMesh(Mesh* mesh, void* userData)
{
	float width = 1;
	float height = 1;
	float length = 1;

	float vertices[] =
	{
		xmin,ymax,zmax,
		xmin,ymin,zmax,
		xmax,ymax,zmax,
		xmax,ymin,zmax,
	};

	float texcoords[] =
	{
		1,0,
		1,1,
		0,0,
		0,1,
	};

	float normals[] =
	{
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
	};

	AllocateMeshData(mesh, 4, 2, 0, true, false);
	memcpy(mesh->vertices, vertices, 4 * 3 * sizeof(float));
	memcpy(mesh->texcoords, texcoords, 4 * 2 * sizeof(float));
	memcpy(mesh->normals, normals, 4 * 3 * sizeof(float));

	mesh->indices[0] = 2;
	mesh->indices[1] = 1;
	mesh->indices[2] = 0;

	mesh->indices[3] = 2;
	mesh->indices[4] = 3;
	mesh->indices[5] = 1;

	mesh->vertexCount = 4;
	mesh->triangleCount = 2;
}

void GenNorthMesh(Mesh* mesh, void* userData)
{
	float width = 1;
	float height = 1;
	float length = 1;

	float vertices[] =
	{
		xmin,ymax,zmin,
		xmin,ymin,zmin,
		xmax,ymax,zmin,
		xmax,ymin,zmin,
	};

	float texcoords[] =
	{
		0,0,
		0,1,
		1,0,
		1,1,
	};

	float normals[] =
	{
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
	};

	AllocateMeshData(mesh, 4, 2, 0, true, false);
	memcpy(mesh->vertices, vertices, 4 * 3 * sizeof(float));
	memcpy(mesh->texcoords, texcoords, 4 * 2 * sizeof(float));
	memcpy(mesh->normals, normals, 4 * 3 * sizeof(float));

	mesh->indices[0] = 0;
	mesh->indices[1] = 1;
	mesh->indices[2] = 2;

	mesh->indices[3] = 1;
	mesh->indices[4] = 3;
	mesh->indices[5] = 2;

	mesh->vertexCount = 4;
	mesh->triangleCount = 2;
}

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

	return ColorFromNormalized(Vector4{ factor, factor, factor,1});
}

void MapRenderer::Setup(CastMap::Ptr map, float scale)
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

	DrawScale = scale;
	MapPointer = map;

	float maxMapSize = Vector2Length(Vector2 { float(map->Width), float(map->Height)});

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
	RenderCells.resize(map->Cells.size());

	std::map<Directions, std::map<size_t, uint16_t>> caches;
	int count = 0;
	for(auto& cell : map->Cells)
	{
		RenderCellVecItr renderCell = RenderCells.begin() + count;
		renderCell->Index = count;

		++count;

		renderCell->MapCell = &cell;
		renderCell->Bounds = Rectangle{ cell.Position.x * DrawScale, cell.Position.y * DrawScale, DrawScale, DrawScale };

		for (auto& cellFace : cell.CellTextures)
		{
			Directions dir = cellFace.first;
			size_t texture = cellFace.second;

			if (caches.find(dir) == caches.end())
				caches[dir] = std::map<size_t, uint16_t>();

			renderCell->RenderFaces[dir] = GetModelFromCache(texture, caches[dir], DirectionMeshes[dir]);
		}
	}
}

RenderCell* MapRenderer::GetCell(int x, int y)
{
	if (MapPointer == nullptr || RenderCells.empty() || x < 0 || x >= MapPointer->Width || y < 0 || y >= MapPointer->Height)
		return nullptr;

	return &(*(RenderCells.begin() + (size_t(y) * size_t(MapPointer->Width) + x)));
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

bool MapRenderer::PointIsOpen(Vector3& postion, float radius)
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
		return false;
 	if (PointInCell(flatPos, radius, GetCell(mapX+1, mapY)))
 		return false;
 	if (PointInCell(flatPos, radius, GetCell(mapX-1, mapY)))
 		return false;
 
 	if (PointInCell(flatPos, radius, GetCell(mapX, mapY + 1)))
 		return false;
 	if (PointInCell(flatPos, radius, GetCell(mapX + 1, mapY + 1)))
 		return false;
 	if (PointInCell(flatPos, radius, GetCell(mapX - 1, mapY + 1)))
 		return false;
 
 	if (PointInCell(flatPos, radius, GetCell(mapX, mapY - 1)))
 		return false;
 	if (PointInCell(flatPos, radius, GetCell(mapX + 1, mapY - 1)))
 		return false;
 	if (PointInCell(flatPos, radius, GetCell(mapX - 1, mapY - 1)))
 		return false;
	
	return true;
}

void MapRenderer::AddVisCell(RenderCell* cell)
{
	if (cell == nullptr)
		return;

	if (VisibleCells.find(cell->Index) == VisibleCells.end())
		VisibleCells.emplace(cell->Index, cell);
}

void MapRenderer::GetTarget(RayCast::Ptr ray, Vector2 &origin)
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
	AddVisCell(GetCell(mapCamera.x-1, mapCamera.y));
	AddVisCell(GetCell(mapCamera.x+1, mapCamera.y));
    AddVisCell(GetCell(mapCamera.x, mapCamera.y-1));
    AddVisCell(GetCell(mapCamera.x - 1, mapCamera.y-1));
    AddVisCell(GetCell(mapCamera.x + 1, mapCamera.y-1));
    AddVisCell(GetCell(mapCamera.x, mapCamera.y+1));
    AddVisCell(GetCell(mapCamera.x - 1, mapCamera.y+1));
    AddVisCell(GetCell(mapCamera.x + 1, mapCamera.y+1));

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

	for (auto &faceListItr : FacesToDraw)
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

static bool DrawDebugRays = false;

void MapRenderer::DrawMiniMap(int posX, int posY, int scale, const Camera& camera, float fovX)
{
	int miniMapWidth = MapPointer->Width * scale;
	int minMapHeight = MapPointer->Height * scale;

	DrawRectangle(posX, posY, miniMapWidth, minMapHeight, GRAY);

	for (int y = 0; y < MapPointer->Height; y++)
	{
		for (int x = 0; x < MapPointer->Width; x++)
		{
			int localX = x * scale + posX;
			int localY = y * scale + posY;

			auto cell = MapPointer->GetCell(x, y);
			DrawRectangle(localX + 1, localY + 1, scale - 1, scale - 1,cell->Solid ? BLACK : DARKGRAY);
		}
	}

	for (auto vis : VisibleCells)
	{
		RenderCell* cell = vis.second;

        int localX = (int)(cell->MapCell->Position.x * scale + posX);
        int localY = (int)(cell->MapCell->Position.y * scale + posY);
        DrawRectangle(localX + 1, localY + 1, scale - 1, scale - 1, LIGHTGRAY);
	}

	Vector2 cameraInMapSpace{ camera.position.x / DrawScale, camera.position.z / DrawScale };
	auto currentCell = GetCell(cameraInMapSpace.x, cameraInMapSpace.y);

	int localX = (int)(cameraInMapSpace.x * scale + posX);
	int localY = (int)(cameraInMapSpace.y * scale + posY);
	DrawCircle(localX, localY, scale / 3.0f, GREEN);

	Vector2 targetVec{camera.target.x - camera.position.x,camera.target.z - camera.position.z };
	targetVec = Vector2Normalize(targetVec);

	int targetX = localX + (int)(targetVec.x * scale * 2);
	int targetY = localY + (int)(targetVec.y * scale * 2);

	DrawLine(localX, localY, targetX, targetY, BLUE);

	Vector2 posFOV = Vector2Rotate(targetVec, fovX * 0.5f);
	targetX = localX + (int)(posFOV.x * scale * 10);
	targetY = localY + (int)(posFOV.y * scale * 10);
	DrawLine(localX, localY, targetX, targetY, GREEN);

	posFOV = Vector2Rotate(targetVec, -fovX * 0.5f);
	targetX = localX + (int)(posFOV.x * scale * 10);
	targetY = localY + (int)(posFOV.y * scale * 10);
	DrawLine(localX, localY, targetX, targetY, GREEN);

	if (DrawDebugRays)
	{
		for (auto& ray : DrawnRays)
		{
			RenderCell* cell = ray.Positive->Target;

			if (cell != nullptr)
			{
				int localX = (int)(cell->MapCell->Position.x * scale + posX);
				int localY = (int)(cell->MapCell->Position.y * scale + posY);
				DrawRectangle(localX + 1, localY + 1, scale - 1, scale - 1, MAROON);
			}

			cell = ray.Negative->Target;

			if (cell != nullptr)
			{
				int localX = (int)(cell->MapCell->Position.x * scale + posX);
				int localY = (int)(cell->MapCell->Position.y * scale + posY);
				DrawRectangle(localX + 1, localY + 1, scale - 1, scale - 1, GOLD);
			}
		}

		for (auto& ray : DrawnRays)
		{
			RenderCell* cell = ray.Positive->Target;

			if (cell != nullptr)
			{
				float len = Vector2Length(Vector2Subtract(cell->MapCell->Position, currentCell->MapCell->Position));

				int targetX = localX + (int)(ray.Positive->Ray.x * scale * len);
				int targetY = localY + (int)(ray.Positive->Ray.y * scale * len);

				DrawLine(localX, localY, targetX, targetY, RED);
			}

			cell = ray.Negative->Target;

			if (cell != nullptr)
			{
				float len = Vector2Length(Vector2Subtract(cell->MapCell->Position, currentCell->MapCell->Position));

				int targetX = localX + (int)(ray.Negative->Ray.x * scale * len);
				int targetY = localY + (int)(ray.Negative->Ray.y * scale * len);

				DrawLine(localX, localY, targetX, targetY, ORANGE);
			}
		}
	}
}

void MapRenderer::DrawMiniMapZoomed(int posX, int posY, int scale, const Camera& camera, float fovX)
{
	int miniMapWidth = 5 * scale;
	int minMapHeight = 5 * scale;

	Vector2 mapCamera{ camera.position.x / DrawScale , camera.position.z / DrawScale };

	int mapX = (int)std::floor(camera.position.x / DrawScale);
	int mapY = (int)std::floor(camera.position.z / DrawScale);

	float centerX = miniMapWidth * 0.5f;
	float centerY = minMapHeight * 0.5f;
	Vector2 targetVec{ camera.target.x - camera.position.x,camera.target.z - camera.position.z };
	targetVec = Vector2Normalize(targetVec);

	Vector2 gridOffset{ mapCamera.x - mapX, mapCamera.y - mapY };
	Vector2 localOffset = Vector2Scale(gridOffset, (float)-scale);

	constexpr Color transColor{ 128,128,128,128 };
	constexpr Color checkedColor{ 128,255,128,128 };
	constexpr Color checkedSolidColor{ 128,128,255,128 };

	for (int y = - 2; y <= +2; ++y)
	{ 
		for (int x = - 2; x <= + 2; ++x)
		{
			const RenderCell* cellPtr = GetCell(mapX + x, mapY + y);
			if (cellPtr == nullptr)
				continue;

			Vector2 gridOffset{ (float)x * scale, (float)y  * scale };

			Color color = cellPtr->checkedForHit ? checkedColor : transColor;
			
			if (cellPtr->MapCell->Solid)
			{
				if (cellPtr->hitCell)
					color = RED;
				else if (cellPtr->checkedForHit)
					color = checkedSolidColor;
				else
					color = BLUE;
			}
			
			if (cellPtr->currentCell)
				color = ORANGE;

			DrawRectangleF(posX + localOffset.x - 1 + centerX + gridOffset.x, posY + localOffset.y - 1 + centerY + gridOffset.y, scale - 1.0f, scale - 1.0f, color);
		}
	}

	DrawCircleF(posX + centerX, posY + centerY, scale / 6.0f, GREEN);
	DrawLineF(posX + centerX, posY + centerY, posX + centerX + (targetVec.x * scale), posY + centerY + (targetVec.y * scale), BLUE);
	Vector2 posFOV = Vector2Scale(Vector2Rotate(targetVec, fovX * 0.5f), scale * 3.0f);
 	DrawLineF(posX + centerX, posY + centerY, posX + centerX + posFOV.x, posY + centerY + posFOV.y, GREEN);

	Vector2 negFov = Vector2Scale(Vector2Rotate(targetVec, -fovX * 0.5f), scale * 3.0f);
	DrawLineF(posX + centerX, posY + centerY, posX + centerX + negFov.x, posY + centerY + negFov.y, GREEN);
}