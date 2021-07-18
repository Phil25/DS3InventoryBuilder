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

	void OnUpdate() override
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

	void OnUpdate(const GridRole role) override
	{
		if (role != GridRole::Inventory)
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

	void OnUpdate(const GridRole destinationRole, const int count) override
	{
		if (destinationRole == GridRole::Inventory)
			inventory->grid->AddSelectedWeapons(count);
		else
			inventory->grid->RemoveSelectedWeapons();
	}
};

class Inventory::InventoryRetriever final : public IInventoryRetriever
{
	Inventory* const inventory;

public:
	InventoryRetriever(Inventory* const inventory) : inventory(inventory)
	{
	}

	auto Get() const -> WeaponContext::WeakVector override
	{
		return inventory->grid->Retrieve();
	}

	void Set(const WeaponContext::Vector& weapons) override
	{
		inventory->grid->Override(weapons);
	}
};

Inventory::Inventory(wxWindow* parent)
	: Title(parent, "Inventory")
	, attributesListener(std::make_shared<AttributesListener>(this))
	, inventorySortingListener(std::make_shared<InventorySortingListener>(this))
	, weaponTransferListener(std::make_shared<WeaponTransferListener>(this))
	, selectionListener(std::make_shared<SelectionListener>(this))
	, inventoryRetriever(std::make_shared<InventoryRetriever>(this))
	, grid(new WeaponGrid(GetContent(), GridRole::Inventory))
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
