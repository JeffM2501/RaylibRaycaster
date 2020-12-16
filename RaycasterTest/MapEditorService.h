#pragma once

#include "Map.h"
#include <functional>
#include <string>
#include <vector>
#include <tuple>

namespace MapEditor
{
	// setup
    void Init(GridMap::Ptr map);
	void SetDefaultTextures(const std::string& wall, const std::string& floor, const std::string& celing);

	// dirty feedback
	typedef std::function<void(int)> CellCallback;
	void SetDirtyCallback(CellCallback callback);

	// undo/redo
	void Undo();
	void Redo();

	bool CanUndo();
	bool CanRedo();

	// selections
	void SelectCell(int index, bool add = false);
	void SelectCells(std::vector<int> indexes, bool add = false);
	void SelectCellFace(int index, Directions dir, bool add = false);
	void ClearCellSelections();
	void ClearFaceSelections();

	bool CellIsSelected(int index);
	bool CellFaceIsSelected(int index, Directions dir);

    std::vector<int> GetSelectedCells();
    std::vector<std::tuple<int, Directions>> GetSelectedFaces();

	// topology
    void SetCellHeights(GridCell* cell, uint8_t floor, uint8_t ceiling);
	void SetCellFloors(GridCell* cell, uint8_t floor);
	void SetCellCeilings(GridCell* cell, uint8_t ceiling);
    void IncrementCellHeights(GridCell* cell, uint8_t floor, uint8_t ceiling);

	void SetCellSolid(GridCell* cell, bool solid);

}