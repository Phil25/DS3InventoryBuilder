#include "Finder.h"

#include <Panels/WeaponGrid.h>

Finder::Finder(wxWindow* parent)
	: Title(parent, "Weapon Finder")
{
	auto* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->AddStretchSpacer(1);
	sizer->Add(new WeaponGrid(GetContent()), 100, wxEXPAND);
	sizer->AddStretchSpacer(1);

	GetContent()->SetSizer(sizer);
}
