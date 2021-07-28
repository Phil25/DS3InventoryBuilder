#include "HelpLinks.h"

HelpLinks::HelpLinks()
{
	//auto* update = new wxMenuItem(this, wxID_ANY, wxT("Update"));
	//update->SetBackgroundColour(wxColor{255,78,80});
	//this->Append(update);

	//this->AppendSeparator();
	this->Append(WebInventoryTool, wxT("DS3 Inventory Tool (web-based)"));
	this->Append(DS3OptimalFashion, wxT("DS3OptimalFashion"));

	this->AppendSeparator();
	this->Append(SoulsPlanner, wxT("SoulsPlanner"));
	this->Append(MugenMonkey, wxT("MugenMonkey"));

	this->AppendSeparator();
	this->Append(WeaponRanges, wxT("Weapon Ranges"));
	this->Append(WeaponCombos, wxT("Weapon Combos"));
	this->Append(PoiseCalculator, wxT("Refined Poise Calculator"));
	this->Append(PoiseResetData, wxT("WA Poise Data"));

	this->AppendSeparator();
	this->Append(TGACETable, wxT("The Grand Archives CE Table"));

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, &HelpLinks::OnSelection, this);
}

void HelpLinks::OnSelection(wxCommandEvent& e)
{
	switch (static_cast<Selection>(e.GetId()))
	{
		case Update:
			wxLaunchDefaultBrowser("");
			break;
		case WebInventoryTool:
			wxLaunchDefaultBrowser("https://ds3-inventory.nyasu.business/");
			break;
		case DS3OptimalFashion:
			wxLaunchDefaultBrowser("https://github.com/Phil25/DS3OptimalFashion/");
			break;
		case SoulsPlanner:
			wxLaunchDefaultBrowser("https://soulsplanner.com/darksouls3");
			break;
		case MugenMonkey:
			wxLaunchDefaultBrowser("https://mugenmonkey.com/darksouls3");
			break;
		case PoiseCalculator:
			wxLaunchDefaultBrowser("https://docs.google.com/spreadsheets/d/1UwIRfNGYqHFM8-53siAn2Q8pVR_STCNp3XslnxibDt4");
			break;
		case PoiseResetData:
			wxLaunchDefaultBrowser("https://docs.google.com/spreadsheets/d/1MadZfjiHI1xIeOmc1cvLNtWPCGnU7Kh4AzyCuAeqA8s");
			break;
		case WeaponCombos:
			wxLaunchDefaultBrowser("https://www.reddit.com/r/darksouls3/comments/66czaw/nikos_true_combo_emporium/");
			break;
		case WeaponRanges:
			wxLaunchDefaultBrowser("https://www.reddit.com/r/darksouls3/comments/67dckl/weapons_attack_ranges_updated_with_dlc2/");
			break;
		case TGACETable:
			wxLaunchDefaultBrowser("https://github.com/inunorii/Dark-Souls-III-CT-TGA/releases/latest");
			break;
		default:
			assert(false && "invalid HelpLinks::Selection");
	}
}
