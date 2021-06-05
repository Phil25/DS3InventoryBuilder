#pragma once

#include <vector>
#include <memory>
#include <string>
#include <Weapon.hpp>

class IAttributesListener;
class IFinderOptionsListener;
class IFinderSortingListener;
class IInventorySortingListener;
class ISelectionListener;
class IWeaponTransferListener;

class WeaponContext final
{
	using Infusion = invbuilder::Weapon::Infusion;

	const int gridID;
	int cardID;

	const std::string name;

	const bool isUnique;
	const bool isInfusable;

	int level;
	Infusion infusion;

public:
	WeaponContext(const int gridID, const int cardID, std::string name, const int level=10, const Infusion=Infusion::None) noexcept;

	auto GetCardID() const noexcept -> int;
	void SetCardID(const int cardID, const int useCount);

	auto GetName() const noexcept -> const std::string&;
	bool IsUnique() const noexcept;

	auto GetLevel(const bool display=false) const noexcept -> int;
	void SetLevel(const int level) noexcept;

	auto GetInfusion() const noexcept -> Infusion;
	void SetInfusion(const Infusion infusion) noexcept;
};

class SessionData final
{
public:
	using SelectionVector = std::vector<std::weak_ptr<WeaponContext>>;
	using Sorting = invbuilder::Weapon::Sorting;

private:
	struct Listeners final
	{
		std::vector<std::weak_ptr<IAttributesListener>> attributes;
		std::vector<std::weak_ptr<IFinderOptionsListener>> finderOptions;
		std::vector<std::weak_ptr<IFinderSortingListener>> finderSorting;
		std::vector<std::weak_ptr<IInventorySortingListener>> inventorySorting;
		std::vector<std::weak_ptr<ISelectionListener>> selection;
		std::vector<std::weak_ptr<IWeaponTransferListener>> weaponTransfer;
	};

	Listeners listeners;

	struct { int str{18}, dex{18}, int_{10}, fth{10}, lck{7}; } attributes;
	Sorting sorting{Sorting::Method::Default, false, false};
	SelectionVector selection;
	int lastSelectionGridID{-1};

public:
	SessionData() = default;

	void UpdateAttributes(const int str, const int dex, const int int_, const int fth, const int lck);
	void UpdateSelection(const int gridID, SelectionVector);
	void UpdateSelection(const int gridID);
	void UpdateSelection();
	void UpdateWeaponTransfer(const int originGridID, const int count);
	void UpdateInventorySorting(Sorting);

	auto GetAttributes() const -> invbuilder::PlayerAttributes;
	auto GetSorting() const -> const Sorting&;
	auto GetSelection() const -> const SelectionVector&;

private:
	void RegisterAttributesListener(const std::weak_ptr<IAttributesListener>&);
	void RegisterFinderOptionsListener(const std::weak_ptr<IFinderOptionsListener>&);
	void RegisterFinderSortingListener(const std::weak_ptr<IFinderSortingListener>&);
	void RegisterInventorySortingListener(const std::weak_ptr<IInventorySortingListener>&);
	void RegisterSelectionListener(const std::weak_ptr<ISelectionListener>&);
	void RegisterWeaponTransferListener(const std::weak_ptr<IWeaponTransferListener>&);

	friend IAttributesListener;
	friend IFinderOptionsListener;
	friend IFinderSortingListener;
	friend IInventorySortingListener;
	friend ISelectionListener;
	friend IWeaponTransferListener;
};