#include "Inventory.h"

#include <Context/ISelectionListener.h>
#include <Context/IWeaponTransferListener.h>

class Inventory::SelectionListener final : public ISelectionListener
{
	Inventory* const inventory;

public:
	SelectionListener(Inventory* const inventory) : inventory(inventory)
	{
	}

	void OnUpdate(const int gridID) override
	{
		if (gridID != inventory->grid->gridID)
			inventory->grid->DiscardSelection();
	}
};

class Inventory::WeaponTransferListener final : public IWeaponTransferListener
{
	Inventory* const inventory;

public:
	WeaponTransferListener(Inventory* const inventory) : inventory(inventory)
	{
	}

	void OnUpdate(const int originGridID, const int count) override
	{
		if (originGridID == inventory->grid->gridID)
			inventory->grid->RemoveSelectedWeapons();

		else
			inventory->grid->AddSelectedWeapons(count);
	}
};

Inventory::Inventory(wxWindow* parent)
	: Title(parent, "Inventory")
	, weaponTransferListener(std::make_shared<WeaponTransferListener>(this))
	, selectionListener(std::make_shared<SelectionListener>(this))
	, grid(new WeaponGrid(GetContent(), false))
{
	weaponTransferListener->Register();
	selectionListener->Register();

	auto* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->AddStretchSpacer(1);
	sizer->Add(grid, 100, wxEXPAND);
	sizer->AddStretchSpacer(1);

	GetContent()->SetSizer(sizer);
}
