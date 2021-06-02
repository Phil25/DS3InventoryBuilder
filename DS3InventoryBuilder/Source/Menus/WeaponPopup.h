#pragma once

#include <wx/menu.h>

class WeaponPopup final : public wxMenu
{
	const int gridID;

public:
	enum Selection
	{
		TransferSingle,
		TransferRow,
		Transfer2Rows,
		TransferPage,
		LevelOffset = 100,
		InfusionOffset = 200,
	};

	WeaponPopup(const int gridID, const bool fixed, const int selectedLevels, const int selectedInfusions);

private:
	void OnSelection(wxCommandEvent&);
	auto GetTransferCount(const Selection) -> int;
};