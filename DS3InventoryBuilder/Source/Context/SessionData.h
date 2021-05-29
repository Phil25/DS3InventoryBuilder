#pragma once

#include <vector>
#include <memory>

class IAttributesListener;
class IFinderOptionsListener;
class IFinderSortingListener;
class IInventorySortingListener;
class ISelectionListener;

class SessionData final
{
	struct Listeners final
	{
		std::vector<std::weak_ptr<IAttributesListener>> attributes;
		std::vector<std::weak_ptr<IFinderOptionsListener>> finderOptions;
		std::vector<std::weak_ptr<IFinderSortingListener>> finderSorting;
		std::vector<std::weak_ptr<IInventorySortingListener>> inventorySorting;
		std::vector<std::weak_ptr<ISelectionListener>> selection;
	};

	Listeners listeners;

	struct { int str, dex, int_, fth, lck; } attributes;

public:
	SessionData() = default;

	void UpdateAttributes(const int str, const int dex, const int int_, const int fth, const int lck);

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