#pragma once

#include <Panels/Title.h>
#include <Panels/WeaponGrid.h>

class Inventory final : public Title
{
	class AttributesListener;
	class InventorySortingListener;
	class WeaponTransferListener;
	class SelectionListener;
	class InventoryRetriever;

	std::shared_ptr<AttributesListener> attributesListener;
	std::shared_ptr<InventorySortingListener> inventorySortingListener;
	std::shared_ptr<WeaponTransferListener> weaponTransferListener;
	std::shared_ptr<SelectionListener> selectionListener;
	std::shared_ptr<InventoryRetriever> inventoryRetriever;

	WeaponGrid* grid;

public:
	Inventory(wxWindow* parent);
};