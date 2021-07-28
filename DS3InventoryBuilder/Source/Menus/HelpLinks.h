#pragma once

#include <wx/menu.h>

class HelpLinks final : public wxMenu
{
	enum Selection
	{
		Update,
		WebInventoryTool,
		DS3OptimalFashion,
		SoulsPlanner,
		MugenMonkey,
		PoiseCalculator,
		PoiseResetData,
		WeaponCombos,
		WeaponRanges,
		TGACETable,
	};

public:
	HelpLinks();

private:
	void OnSelection(wxCommandEvent& e);
};