#pragma once

#include <wx/menu.h>

class WeaponPopup final : public wxMenu
{
	enum Selection
	{
		TransferSingle,
		TransferRow,
		Transfer2Rows,
		TransferPage,
		SelectAll,
		LevelOffset = 100,
		InfusionOffset = 200,
	};

	const int gridID;
	Selection selection;

public:
	WeaponPopup(const int gridID, const bool fixed, const int selectedLevels, const int selectedInfusions);

	bool ShouldSelectAll() const;

private:
	void OnSelection(wxCommandEvent&);

	static inline auto GetTransferCount(const Selection) -> int;
};