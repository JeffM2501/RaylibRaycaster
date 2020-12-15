#include "MapEditorService.h"

#include <vector>
#include <tuple>
#include <algorithm>

namespace MapEditor
{
	CellCallback DirtyCallback = nullptr;

	std::string DefaultWallTexture;
	std::string DefaultFloorTexture;
	std::string DefaultCeilingtexture;

	std::vector<int> DirtyCells;

	GridMap::Ptr MapPointer = nullptr;

    void Init(GridMap::Ptr map)
    {
        DirtyCells.clear();
        MapPointer = map;
    }

	void SetDefaultTextures(const std::string& wall, const std::string& floor, const std::string& celing)
	{
		DefaultWallTexture = wall;
		DefaultFloorTexture = floor;
		DefaultCeilingtexture = celing;
	}

	void SetDirtyCallback(CellCallback callback)
	{
		DirtyCallback = callback;
	}

	void AddDirty(int cell)
	{
		if (std::find(DirtyCells.begin(), DirtyCells.end(), cell) == DirtyCells.end())
			DirtyCells.emplace_back(cell);
	}

	void AddDirty(const std::vector<int>& cells)
	{
		for (int cell : cells)
			AddDirty(cell);
	}

	bool CellNeedsWall(GridCell* cell, Directions dir)
	{
		if (cell->IsSolid())
			return false;

		GridCell *otherCell = MapPointer->GetDirectionCell(cell, dir);
		if (otherCell == nullptr)
			return false;

		if (otherCell->IsSolid())
			return true;

		if (otherCell->GetFloorValue() <= cell->GetFloorValue() && otherCell->GetCeilingValue() >= cell->GetCeilingValue())
			return false;

		return true;
	}

	bool CheckFaceWall(GridCell* cell, Directions dir)
	{
		std::map<Directions, size_t>::iterator itr = cell->CellTextures.find(dir);

		if (CellNeedsWall(cell, dir))
		{
			if (itr == cell->CellTextures.end())
			{
				cell->CellTextures[dir] = MapPointer->AddMaterial(DefaultWallTexture);
				return true;
			}
				
		}
		else
		{
			if (itr != cell->CellTextures.end())
			{
				cell->CellTextures.erase(itr);
				return true;
			}
		}

		return false;
	}

	std::vector<int> CheckFacesAround(int index)
	{
		std::vector<int> newDirty;

		GridCell* cell = MapPointer->GetCell(index);
		if (cell == nullptr)
			return newDirty;

		CheckFaceWall(cell, Directions::XNeg);
		CheckFaceWall(cell, Directions::XPos);
		CheckFaceWall(cell, Directions::ZNeg);
		CheckFaceWall(cell, Directions::ZPos);

		if (cell->IsSolid())
		{
			std::map<Directions, size_t>::iterator itr = cell->CellTextures.find(Directions::YNeg);
			if (itr != cell->CellTextures.end())
				cell->CellTextures.erase(itr);

			itr = cell->CellTextures.find(Directions::YPos);
			if (itr != cell->CellTextures.end())
				cell->CellTextures.erase(itr);
		}
		else
		{
			std::map<Directions, size_t>::iterator itr = cell->CellTextures.find(Directions::YNeg);
			if (itr == cell->CellTextures.end())
				cell->CellTextures[Directions::YNeg] = MapPointer->AddMaterial(DefaultFloorTexture);

			itr = cell->CellTextures.find(Directions::YPos);
			if (itr != cell->CellTextures.end())
				cell->CellTextures[Directions::YPos] = MapPointer->AddMaterial(DefaultCeilingtexture);
		}

		CheckFaceWall(MapPointer->GetCell(cell->Position.x + 1, cell->Position.y), Directions::XNeg);
		newDirty.push_back(MapPointer->GetCell(cell->Position.x + 1, cell->Position.y)->Index);

		CheckFaceWall(MapPointer->GetCell(cell->Position.x - 1, cell->Position.y), Directions::XPos);
		newDirty.push_back(MapPointer->GetCell(cell->Position.x - 1, cell->Position.y)->Index);

		CheckFaceWall(MapPointer->GetCell(cell->Position.x, cell->Position.y + 1), Directions::ZNeg);
		newDirty.push_back(MapPointer->GetCell(cell->Position.x, cell->Position.y + 1)->Index);

		CheckFaceWall(MapPointer->GetCell(cell->Position.x, cell->Position.y - 1), Directions::ZPos);
		newDirty.push_back(MapPointer->GetCell(cell->Position.x, cell->Position.y - 1)->Index);

		return newDirty;
	}

	void InvokeDirty()
	{
		std::vector<int> currentDirty = DirtyCells;
		for (auto cell : currentDirty)
		{
			auto newDirty = CheckFacesAround(cell);
			for (auto d : newDirty)
				AddDirty(d);
		}

		if (DirtyCallback != nullptr)
		{
			for (auto cell : DirtyCells)
				DirtyCallback(cell);
		}

		DirtyCells.clear();
	}


	// Actions

	class EditAction
	{
	public:
		using Ptr = std::shared_ptr<EditAction>;

		virtual std::vector<int> Undo(GridMap::Ptr map) = 0;
		virtual std::vector<int> Redo(GridMap::Ptr map) = 0;
	};

	std::vector<EditAction::Ptr> UndoBuffer;
	size_t UndoIndex = 0;

	void Undo()
	{
		if (!CanUndo())
			return;
		AddDirty(UndoBuffer[UndoIndex-1]->Undo(MapPointer));
		UndoIndex--;
		InvokeDirty();
	}

	void Redo()
	{
		if (!CanRedo())
			return;
		AddDirty(UndoBuffer[UndoIndex]->Redo(MapPointer));
		UndoIndex++;
		InvokeDirty();
	}

	bool CanUndo()
	{
		return UndoBuffer.size() > 0 && UndoIndex > 0;
	}

	bool CanRedo()
	{
		return UndoBuffer.size() > 0 && UndoIndex < UndoBuffer.size();
	}

    void PushEdit(EditAction::Ptr action)
    {
        AddDirty(action->Redo(MapPointer));
        UndoBuffer.resize(UndoIndex);
        UndoBuffer.emplace_back(action);
        ++UndoIndex;
        InvokeDirty();
    }

	// selections

	std::vector<int> SelectedCells;
	std::vector<std::tuple<int, Directions>> SelectedFaces;

	void ClearCellSelections()
	{
		SelectedCells.clear();
	}

	void ClearFaceSelections()
	{
		SelectedFaces.clear();
	}

	void SelectCell(int index, bool add)
	{
		if (!add)
			ClearCellSelections();

		if (!CellIsSelected(index))
			SelectedCells.push_back(index);
	}

	void SelectCells(std::vector<int> indexes, bool add)
	{
        if (!add)
            ClearCellSelections();

		for (auto index : indexes)
		{
            if (!CellIsSelected(index))
                SelectedCells.push_back(index);
		}
	}

	void SelectCellFace(int index, Directions dir, bool add)
	{
        if (!add)
			ClearFaceSelections();

		if (!CellFaceIsSelected(index,dir))
			SelectedFaces.push_back(std::tuple<int, Directions>(index, dir));
	}

	bool CellIsSelected(int index)
	{
		return std::find(SelectedCells.begin(), SelectedCells.end(), index) != SelectedCells.end();
	}

	bool CellFaceIsSelected(int index, Directions dir)
	{
        return std::find(SelectedFaces.begin(), SelectedFaces.end(), std::tuple<int, Directions>(index, dir)) != SelectedFaces.end();
    }

	std::vector<int> GetSelectedCells()
	{
		return SelectedCells;
	}

	std::vector<std::tuple<int, Directions>> GetSelectedFaces()
	{
		return SelectedFaces;
	}

	// topology
	class SetCellHeight : public EditAction
	{
	public:
		using Ptr = std::shared_ptr<SetCellHeight>;
		static Ptr Create() { return std::make_shared<SetCellHeight>(); }

		using EditFace = std::tuple<Directions, int, uint8_t>;
		std::vector<EditFace> Edits;

		std::vector<EditFace> PreviousState;

		std::vector<int> Redo(GridMap::Ptr map) override
		{
			std::vector<int> affectedCells;
			PreviousState.clear();

			for (auto& f : Edits)
			{
				Directions dir = std::get<0>(f);
				int id = std::get<1>(f);
				auto cell = map->GetCell(id);
				if (cell != nullptr)
				{
					if (dir == Directions::YNeg)
					{
						PreviousState.emplace_back(EditFace(dir, id, cell->Floor));
						cell->Floor = std::get<2>(f);
					}
					else
					{
						PreviousState.emplace_back(EditFace(dir, id, cell->Ceiling));
						cell->Ceiling = std::get<2>(f);
					}

					affectedCells.push_back(id);
				}
			}

			return affectedCells;
		}

		std::vector<int> Undo(GridMap::Ptr map) override
		{
			std::vector<int> affectedCells;
			for (auto& f : PreviousState)
			{
				Directions dir = std::get<0>(f);
				int id = std::get<1>(f);
				auto cell = map->GetCell(id);
				if (cell != nullptr)
				{
					if (dir == Directions::YNeg)
					{
						cell->Floor = std::get<2>(f);
					}
					else
					{
						cell->Ceiling = std::get<2>(f);
					}

					affectedCells.push_back(id);
				}
			}

			return affectedCells;
		}
	};

	void AddCellHeightChange(SetCellHeight::Ptr command, GridCell* cell, uint8_t floor, uint8_t ceiling)
	{
       
	}

	bool DoForeachSelectedCell(GridCell* cell, std::function<void(GridCell*)> func)
	{
		if (func == nullptr || MapPointer == nullptr)
			return false;

		if (cell != nullptr)
			func(cell);
		else
		{
			if (SelectedCells.size() == 0)
				return false;
			for (auto index : SelectedCells)
			{
				GridCell* c = MapPointer->GetCell(index);
				if (c != nullptr)
					func(c);
			}
		}

		return true;
	}

	void SetCellHeights(GridCell* cell, uint8_t floor, uint8_t ceiling)
	{
		SetCellHeight::Ptr command = SetCellHeight::Create();

		if (DoForeachSelectedCell(cell, [command, floor, ceiling](GridCell* cellptr)
			{
                command->Edits.push_back(SetCellHeight::EditFace(Directions::YNeg, cellptr->Index, floor));
                command->Edits.push_back(SetCellHeight::EditFace(Directions::YPos, cellptr->Index, ceiling));
			}))
		PushEdit(command);
	}

	void IncrementCellHeights(GridCell* cell, uint8_t floor, uint8_t ceiling)
	{
        SetCellHeight::Ptr command = SetCellHeight::Create();

        if (DoForeachSelectedCell(cell, [command, floor, ceiling](GridCell* cellptr)
            {
                command->Edits.push_back(SetCellHeight::EditFace(Directions::YNeg, cellptr->Index, cellptr->Floor + floor));
                command->Edits.push_back(SetCellHeight::EditFace(Directions::YPos, cellptr->Index, cellptr->Ceiling + ceiling));
            }))
        PushEdit(command);
	}
}