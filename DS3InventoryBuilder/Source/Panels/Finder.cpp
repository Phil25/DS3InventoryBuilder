#include "Finder.h"

#include <Panels/WeaponGrid.h>

Finder::Finder(wxWindow* parent)
	: Title(parent, "Weapon Finder")
{
	auto* weaponGrid = new WeaponGrid(GetContent());
	weaponGrid->InitializeBaseWeapons();

	auto* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->AddStretchSpacer(1);
	sizer->Add(weaponGrid, 100, wxEXPAND);
	sizer->AddStretchSpacer(1);

	GetContent()->SetSizer(sizer);
}
