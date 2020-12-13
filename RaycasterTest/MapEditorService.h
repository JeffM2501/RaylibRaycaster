#pragma once

#include "Map.h"
#include <functional>
#include <string>

namespace MapEditor
{
	typedef std::function<void(int)> CellCallback;

	void SetDirtyCallback(CellCallback callback);

	void SetDefaultTextures(const std::string& wall, const std::string& floor, const std::string& celing);

	void Init(GridMap::Ptr map);

	void SetCellHeights(GridCell* cell, uint8_t floor, uint8_t ceiling);

	void Undo();
	void Redo();

	bool CanUndo();
	bool CanRedo();
}