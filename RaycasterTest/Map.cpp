#include "Map.h"
#include "ResourceManager.h"
#include "raymath.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

GridMap::~GridMap()
{
}

bool PixelIsSolid(int x, int y, Color* imageData, int width)
{
	return (imageData + (y * width + x))->b > 0;
}

void GridMap::LoadFromImage(const Image& image, float scale, size_t walls, size_t floor, size_t ceiling)
{
	Size.x = image.width;
	Size.y = image.height;

	Cells.clear();

	Cells.resize(size_t(Size.x) * size_t(Size.y));
	Color* imageData = GetImageData(image);

	for (int y = 0; y < Size.y; y++)
	{
		for (int x = 0; x < Size.x; x++)
		{
			std::vector<GridCell>::iterator cell = Cells.begin() + (size_t(y) * Size.x + x);
			cell->Position.x = x;
			cell->Position.y = y;
			cell->Floor = PixelIsSolid(x, y, imageData, Size.x) ? 255 : 128;
			if (!cell->IsSolid())
			{
				if (x != 0 && PixelIsSolid(x - 1, y, imageData, Size.x))			// west side is closed, add a wall
					cell->CellTextures[Directions::XNeg] = walls;
 
 				if (x != Size.x -1 && PixelIsSolid(x + 1, y, imageData, Size.x))	// east side is closed, add a wall
 					cell->CellTextures[Directions::XPos] = walls;

				if (y != 0 && PixelIsSolid(x, y - 1, imageData, Size.x))			// north side is closed, add a wall
					cell->CellTextures[Directions::ZNeg] = walls;

				if (y != Size.y - 1 && PixelIsSolid(x, y+1, imageData, Size.x))	// east side is closed, add a wall
					cell->CellTextures[Directions::ZPos] = walls;

				cell->CellTextures[Directions::YNeg] = floor;
				cell->CellTextures[Directions::YPos] = ceiling;
			}
		}
	}
	FreeData(imageData);
}

size_t GridMap::AddMaterial(const std::string& path)
{
	size_t maxId = 0;
	for (auto& mat : MaterialList)
	{
		if (mat.second == path)
			return mat.first;

		if (mat.first > maxId)
			maxId = mat.first;
	}

	++maxId;
	MaterialList[maxId] = path;
	if (MaterialAdded != nullptr)
		MaterialAdded(maxId, path);

	return maxId;
}

constexpr int MapFileMagic = 0x0F0F0F0F;
constexpr int MapFileVersion = 1;

bool GridMap::LoadFromFile(const std::string& path)
{
	MaterialList.clear();
	Cells.clear();
	Size.x = 0;
	Size.y = 0;

	FILE* fp = nullptr;

	bool valid = false;

	try
	{
		fopen_s(&fp, path.c_str(), "rb");
		if (fp == nullptr)
			throw;

		int magic = 0;
		fread(&magic, sizeof(int), 1, fp);

		int version = 0;
		fread(&version, sizeof(int), 1, fp);

		if (magic == MapFileMagic && version == MapFileVersion)
		{
			fread(&Size, sizeof(int), 2, fp);

			int matCount = 0;
			fread(&matCount, sizeof(int), 1, fp);

			for (int i = 0; i < matCount; ++i)
			{
				int id = 0;
				fread(&id, sizeof(int), 1, fp);

				int size = 0;
				fread(&size, sizeof(int), 1, fp);
				char* name = new char[(size_t)size+1];
				fread(name, size, 1, fp);
				name[size] = '\0';
				MaterialList[id] = name;

				delete[](name);
			}

			int cellCount = Size.x * Size.y;
			Cells.resize(cellCount);
			for (int i = 0; i < cellCount; ++i)
			{
				GridCell &cell = Cells[i];
				cell.Index = i;
				cell.Position.y = i / Size.x;
				cell.Position.x = i - (cell.Position.y * Size.x);
				
				fread(&cell.Floor, sizeof(uint8_t), 1, fp);
				fread(&cell.Ceiling, sizeof(uint8_t), 1, fp);

				int size = 0;
				fread(&size, sizeof(int), 1, fp);
				for (int f = 0; f < size; ++f)
				{
					uint8_t dir = 0;
					fread(&dir, sizeof(uint8_t), 1, fp);

					int matId = 0;
					fread(&matId, sizeof(int), 1, fp);

					cell.CellTextures.emplace((Directions)dir, matId);
				}
			}

			valid = true;
		}
	}
	catch (...)
	{
			
	}
	if (fp != nullptr)
		fclose(fp);

	return valid;
}


void GridMap::SaveToFile(const std::string& path)
{
	FILE* fp = nullptr;
	try
	{
		fopen_s(&fp, path.c_str(), "w+b");
		if (fp == nullptr)
			throw;

		// header
		fwrite(&MapFileMagic, sizeof(int), 1, fp);
		fwrite(&MapFileVersion, sizeof(int), 1, fp);

		// map sizes
		fwrite(&Size, sizeof(int), 2, fp);

		// material indexes
		int size = (int)MaterialList.size();
		fwrite(&size, sizeof(int), 1, fp);
		for (auto& mat : MaterialList)
		{
			size = (int)mat.first;
			fwrite(&size, sizeof(int), 1, fp);

			size = (int)mat.second.size();
			fwrite(&size, sizeof(int), 1, fp);

			fwrite(mat.second.c_str(), size, 1, fp);
		}

		for (auto& cell : Cells)
		{
			fwrite(&cell.Floor, sizeof(uint8_t), 1, fp);
			fwrite(&cell.Ceiling, sizeof(uint8_t), 1, fp);

			size = (int)cell.CellTextures.size();
			fwrite(&size, sizeof(int), 1, fp);
			for (auto face : cell.CellTextures)
			{
				uint8_t d = (uint8_t)face.first;
				fwrite(&d, sizeof(uint8_t), 1, fp);

				size = (int)face.second;
				fwrite(&size, sizeof(int), 1, fp);
			}
		}
	}
	catch (...)
	{
		
	}
	if (fp != nullptr)
		fclose(fp);
}

GridCell* GridMap::GetCell(int x, int y)
{
	if (Cells.empty() || x < 0 || x >= Size.x || y < 0 || y >= Size.y)
		return nullptr;

	return &(*(Cells.begin() + (size_t(y) * size_t(Size.x) + x)));
}

GridCell* GridMap::GetCell(int index)
{
	if (Cells.empty() || index < 0 || index >= Cells.size())
		return nullptr;

	return &(*(Cells.begin() + index));
}

GridCell* GridMap::GetDirectionCell(GridCell* sourceCell, Directions dir)
{
	if (sourceCell == nullptr)
		return nullptr;

	int xOffset = dir == Directions::XPos ? 1 : dir == Directions::XNeg ? -1 : 0;
	int yOffset = dir == Directions::ZPos ? 1 : dir == Directions::ZNeg ? -1 : 0;

	return GetCell(((sourceCell->Position.y + yOffset) * Size.x) + sourceCell->Position.x + xOffset);
}

GridCell* GridMap::GetDirectionCell(int sourceIndex, Directions dir)
{
	return GetDirectionCell(GetCell(sourceIndex), dir);
}

void GridMap::DoForEachCell(std::function<void(GridCell* cell)> func)
{
	if (func == nullptr)
		return;

	for (GridCell& cell : Cells)
	{
		func(&cell);
	}
}
