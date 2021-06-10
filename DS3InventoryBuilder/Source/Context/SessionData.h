#pragma once

#include <memory>
#include <string>
#include <Weapon.hpp>
#include <Context/WeaponContext.h>

class IAttributesListener;
class IInventorySortingListener;
class ISelectionListener;
class IWeaponTransferListener;
class IInventoryRetriever;

class SessionData final
{
	using Sorting = invbuilder::Weapon::Sorting;

	struct Listeners final
	{
		std::vector<std::weak_ptr<IAttributesListener>> attributes;
		std::vector<std::weak_ptr<IInventorySortingListener>> inventorySorting;
		std::vector<std::weak_ptr<ISelectionListener>> selection;
		std::vector<std::weak_ptr<IWeaponTransferListener>> weaponTransfer;
	};

	Listeners listeners;
	std::weak_ptr<IInventoryRetriever> inventoryRetriever;

	struct { int str{18}, dex{18}, int_{10}, fth{10}, lck{7}; } attributes;
	Sorting sorting{Sorting::Method::Default, false, false};
	WeaponContext::WeakVector selection;
	int lastSelectionGridID{-1};

public:
	SessionData() = default;

	void UpdateAttributes(const int str, const int dex, const int int_, const int fth, const int lck);
	void UpdateSelection(const int gridID, WeaponContext::WeakVector);
	void UpdateSelection(const int gridID);
	void UpdateSelection();
	void UpdateWeaponTransfer(const int originGridID, const int count);
	void UpdateInventorySorting(Sorting);

	void OverrideWeapons(const WeaponContext::Vector&);

	auto GetInventory() const -> WeaponContext::WeakVector;
	auto GetAttributes() const -> invbuilder::PlayerAttributes;
	auto GetSorting() const -> const Sorting&;
	auto GetSelection() const -> const WeaponContext::WeakVector&;

private:
	void RegisterAttributesListener(const std::weak_ptr<IAttributesListener>&);
	void RegisterInventorySortingListener(const std::weak_ptr<IInventorySortingListener>&);
	void RegisterSelectionListener(const std::weak_ptr<ISelectionListener>&);
	void RegisterWeaponTransferListener(const std::weak_ptr<IWeaponTransferListener>&);
	void RegisterInventoryRetriever(std::weak_ptr<IInventoryRetriever>);

	friend IAttributesListener;
	friend IInventorySortingListener;
	friend ISelectionListener;
	friend IWeaponTransferListener;
	friend IInventoryRetriever;
};