#include "Inventory.h"

#include <iostream> // TODO
#include <Context/IWeaponTransferListener.h>

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
	, grid(new WeaponGrid(GetContent(), false))
{
	weaponTransferListener->Register();

	auto* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->AddStretchSpacer(1);
	sizer->Add(grid, 100, wxEXPAND);
	sizer->AddStretchSpacer(1);

	GetContent()->SetSizer(sizer);
}
