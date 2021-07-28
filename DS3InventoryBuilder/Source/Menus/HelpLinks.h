#pragma once

#include <wx/menu.h>

class HelpLinks final : public wxMenu
{
	enum Selection
	{
		Update,

		// promo
		WebInventoryTool,
		DS3OptimalFashion,

		// character
		SoulsPlanner,
		MugenMonkey,

		// spreadsheets
		PoiseCalculator,
		PoiseResetData,
		MotionValues,
		WeaponArtGroups,
		ParryData,
		MiscData,

		// reddit
		WeaponCombos,
		WeaponRanges,

		// cheat
		TGACETable,
	};

public:
	HelpLinks(const bool hasLatestVersion);

private:
	void OnSelection(wxCommandEvent& e);
};