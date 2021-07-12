#pragma once

#include <wx/menu.h>
#include <Utils/GridRole.hpp>

class WeaponPopup final : public wxMenu
{
	enum Selection
	{
		TransferSingle,
		TransferRow,
		Transfer2Rows,
		TransferPage,
		DuplicateSingle,
		DuplicateRow,
		Duplicate2Rows,
		DuplicatePage,
		SelectAll,
		LevelOffset = 100,
		InfusionOffset = 200,
	};

	const GridRole role;
	Selection selection;

public:
	WeaponPopup(const GridRole role, const int selectedLevels, const int selectedInfusions);

	bool ShouldSelectAll() const;
	bool WereWeaponsAltered() const;
	bool WereWeaponsTransferred() const;

private:
	void OnSelection(wxCommandEvent&);

	static inline auto CreateLevelSubmenu(const int selected) -> wxMenu*;
	static inline auto CreateInfusionSubmenu(const int selected) -> wxMenu*;

	static inline auto GetTransferCount(const Selection) -> int;
};