#include "Inventory.h"

#include <AppMain.h>
#include <Context/IAttributesListener.h>
#include <Context/IInventorySortingListener.h>
#include <Context/ISelectionListener.h>
#include <Context/IWeaponTransferListener.h>
#include <Context/IInventoryRetriever.h>

class Inventory::AttributesListener final : public IAttributesListener
{
	Inventory* const inventory;

public:
	AttributesListener(Inventory* const inventory) : inventory(inventory)
	{
	}

	void OnUpdate(const int str, const int dex, const int int_, const int fth, const int lck) override
	{
		using Method = invbuilder::Weapon::Sorting::Method;

		if (wxGetApp().GetSessionData().GetSorting().method == Method::AttackPower)
			inventory->grid->Sort();

		inventory->grid->UpdateRequirements();
	}
};

class Inventory::InventorySortingListener final : public IInventorySortingListener
{
	Inventory* const inventory;

public:
	InventorySortingListener(Inventory* const inventory) : inventory(inventory)
	{
	}

	void OnUpdate(const invbuilder::Weapon::Sorting&) override
	{
		inventory->grid->Sort();
	}
};

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

class Inventory::InventoryRetriever final : public IInventoryRetriever
{
	Inventory* const inventory;

public:
	InventoryRetriever(Inventory* const inventory) : inventory(inventory)
	{
	}

	auto Get() const -> WeaponContext::Vector override
	{
		return inventory->grid->Retrieve();
	}
};

Inventory::Inventory(wxWindow* parent)
	: Title(parent, "Inventory")
	, attributesListener(std::make_shared<AttributesListener>(this))
	, inventorySortingListener(std::make_shared<InventorySortingListener>(this))
	, weaponTransferListener(std::make_shared<WeaponTransferListener>(this))
	, selectionListener(std::make_shared<SelectionListener>(this))
	, inventoryRetriever(std::make_shared<InventoryRetriever>(this))
	, grid(new WeaponGrid(GetContent(), false))
{
	attributesListener->Register();
	inventorySortingListener->Register();
	weaponTransferListener->Register();
	selectionListener->Register();
	inventoryRetriever->Register();

	auto* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->AddStretchSpacer(1);
	sizer->Add(grid, 100, wxEXPAND);
	sizer->AddStretchSpacer(1);

	GetContent()->SetSizer(sizer);
}
