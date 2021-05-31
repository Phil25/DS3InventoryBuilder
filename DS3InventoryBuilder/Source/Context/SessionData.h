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

class WeaponContext final
{
	using Infusion = invbuilder::Weapon::Infusion;

	int level;
	Infusion infusion;

public:
	const int gridID;
	const int cardID;
	const std::string name;

	WeaponContext(const int gridID, const int cardID, std::string name, const int level=10, const Infusion=Infusion::None) noexcept;

	auto GetLevel() const noexcept -> int;
	void SetLevel(const int level) noexcept;

	auto GetInfusion() const noexcept -> Infusion;
	void SetInfusion(const Infusion infusion) noexcept;
};

class SessionData final
{
public:
	using SelectionVector = std::vector<std::weak_ptr<WeaponContext>>;

private:
	struct Listeners final
	{
		std::vector<std::weak_ptr<IAttributesListener>> attributes;
		std::vector<std::weak_ptr<IFinderOptionsListener>> finderOptions;
		std::vector<std::weak_ptr<IFinderSortingListener>> finderSorting;
		std::vector<std::weak_ptr<IInventorySortingListener>> inventorySorting;
		std::vector<std::weak_ptr<ISelectionListener>> selection;
	};

	Listeners listeners;

	struct { int str{18}, dex{18}, int_{10}, fth{10}, lck{7}; } attributes;
	SelectionVector selection;

public:
	SessionData() = default;

	void UpdateAttributes(const int str, const int dex, const int int_, const int fth, const int lck);
	void UpdateSelection(SelectionVector);

	auto GetAttributes() const -> const invbuilder::PlayerAttributes&;
	auto GetSelection() const -> const SelectionVector&;

private:
	void RegisterAttributesListener(const std::weak_ptr<IAttributesListener>&);
	void RegisterFinderOptionsListener(const std::weak_ptr<IFinderOptionsListener>&);
	void RegisterFinderSortingListener(const std::weak_ptr<IFinderSortingListener>&);
	void RegisterInventorySortingListener(const std::weak_ptr<IInventorySortingListener>&);
	void RegisterSelectionListener(const std::weak_ptr<ISelectionListener>&);

	friend IAttributesListener;
	friend IFinderOptionsListener;
	friend IFinderSortingListener;
	friend IInventorySortingListener;
	friend ISelectionListener;
};