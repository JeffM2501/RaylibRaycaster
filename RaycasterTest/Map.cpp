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
	DirectionMeshes[Directions::XNeg] = GenMeshCustom(&GenWestMesh, nullptr);
	DirectionMeshes[Directions::XPos] = GenMeshCustom(&GenEastMesh, nullptr);

	DirectionMeshes[Directions::ZNeg] = GenMeshCustom(&GenNorthMesh, nullptr);
	DirectionMeshes[Directions::ZPos] = GenMeshCustom(&GenSouthMesh, nullptr);

	DirectionMeshes[Directions::YPos] = GenMeshCustom(&GenCeilingMesh, nullptr);
	DirectionMeshes[Directions::YNeg] = GenMeshCustom(&GenFloorMesh, nullptr);
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
	DrawScale = scale;
	MapPointer = map;

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
	size_t count = 0;
	for(auto& cell : map->Cells)
	{
		RenderCellVecItr renderCell = RenderCells.begin() + count;
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

bool MapRenderer::PointInCell(Vector2& postion, float radius, RenderCell* cellPtr)
{
	if (cellPtr == nullptr || cellPtr->MapCell == nullptr)
		return true;

	cellPtr->checkedForHit = true;

	if (!cellPtr->MapCell->Solid)
		return false;

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

void MapRenderer::Draw()
{
	Vector3 pos{ 0,0, 0};
	Vector3 YAxis{ 0,1,0 };
	Vector3 XAxis{ 1,0,0 };
	Vector3 scale{ DrawScale,DrawScale,DrawScale };

 	for (int y = 0; y < MapPointer->Height; y++)
 	{
 		for (int x = 0; x < MapPointer->Width; x++)
 		{
 			pos.x = x * DrawScale;
 			pos.z = y * DrawScale;
 
 			RenderCellVecItr itr = RenderCells.begin() + (size_t(y) * MapPointer->Width) + x;
 			for (auto& face : itr->RenderFaces)
 			{
				if (face.first == Directions::YPos)
					pos.y = DrawScale;
				else
					pos.y = 0;

 				DrawModel(ModelCache[face.second], pos, DrawScale, DirectionColors[face.first]);
 			}

			// debug bounds
			if (itr->MapCell->Solid)
				DrawRect3DXZ(itr->Bounds, 0.1f, itr->MapCell->Solid ? RED : LIME);
 		}
 	}
}

void MapRenderer::DrawMiniMap(int posX, int posY, int scale, Camera& camera, float fovX)
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

			DrawRectangle(localX + 1, localY + 1, scale - 1, scale - 1, MapPointer->GetCell(x, y)->CellTextures.empty() ? BLACK : DARKGRAY);
		}
	}
	int localX = (int)(camera.position.x * scale + posX);
	int localY = (int)(camera.position.z * scale + posY);
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
}

void MapRenderer::DrawMiniMapZoomed(int posX, int posY, int scale, Camera& camera, float fovX)
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