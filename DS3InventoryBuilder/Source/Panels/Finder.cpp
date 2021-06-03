#include "Finder.h"

#include <Context/ISelectionListener.h>
#include <Panels/WeaponGrid.h>

class Finder::SelectionListener final : public ISelectionListener
{
	Finder* const finder;

public:
	SelectionListener(Finder* const finder) : finder(finder)
	{
	}

	void OnUpdate(const int gridID) override
	{
		if (gridID != finder->grid->gridID)
			finder->grid->DiscardSelection();
	}
};

Finder::Finder(wxWindow* parent)
	: Title(parent, "Weapon Finder")
	, selectionListener(std::make_shared<SelectionListener>(this))
	, grid(new WeaponGrid(GetContent()))
{
	selectionListener->Register();

	grid->InitializeBaseWeapons();

	auto* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->AddStretchSpacer(1);
	sizer->Add(grid, 100, wxEXPAND);
	sizer->AddStretchSpacer(1);

	GetContent()->SetSizer(sizer);
}
