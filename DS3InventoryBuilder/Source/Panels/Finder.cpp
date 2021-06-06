#include "Finder.h"

#include <Context/ISelectionListener.h>
#include <Context/IAttributesListener.h>
#include <Panels/WeaponGrid.h>

class Finder::AttributesListener final : public IAttributesListener
{
	Finder* const finder;

public:
	AttributesListener(Finder* const finder) : finder(finder)
	{
	}

	void OnUpdate(const int str, const int dex, const int int_, const int fth, const int lck) override
	{
		finder->grid->UpdateRequirements();
	}
};

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
	, attributesListener(std::make_shared<AttributesListener>(this))
	, selectionListener(std::make_shared<SelectionListener>(this))
	, grid(new WeaponGrid(GetContent()))
{
	attributesListener->Register();
	selectionListener->Register();

	grid->InitializeBaseWeapons();

	auto* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->AddStretchSpacer(1);
	sizer->Add(grid, 100, wxEXPAND);
	sizer->AddStretchSpacer(1);

	GetContent()->SetSizer(sizer);
}
