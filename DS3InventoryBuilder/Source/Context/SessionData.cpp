#include "SessionData.h"

#include <AppMain.h>
#include <Context/IAttributesListener.h>
#include <Context/IInventorySortingListener.h>
#include <Context/ISelectionListener.h>
#include <Context/IWeaponTransferListener.h>
#include <Context/IInventoryRetriever.h>
#include <cassert>

void SessionData::UpdateAttributes(const int str, const int dex, const int int_, const int fth, const int lck)
{
	assert(1 <= str && str <= 99 && "str value outside legal range");
	assert(1 <= dex && dex <= 99 && "dex value outside legal range");
	assert(1 <= int_ && int_ <= 99 && "int_ value outside legal range");
	assert(1 <= fth && fth <= 99 && "fth value outside legal range");
	assert(1 <= lck && lck <= 99 && "lck value outside legal range");

	if (str == attributes.str && dex == attributes.dex && int_ == attributes.int_ && fth == attributes.fth && lck == attributes.lck)
		return;

	attributes.str = str;
	attributes.dex = dex;
	attributes.int_ = int_;
	attributes.fth = fth;
	attributes.lck = lck;

	UpdateSelection();

	for (const auto& weakPtr : listeners.attributes)
		if (const auto ptr = weakPtr.lock(); ptr)
			ptr->OnUpdate();
}

void SessionData::UpdateSelection(const GridRole role, std::vector<std::weak_ptr<WeaponContext>> selection)
{
	if (selection.empty() && role != lastGridRole)
		return; // empty update from irrelevant grid

	this->selection = std::move(selection);

	lastGridRole = role;
	UpdateSelection(role);
}

void SessionData::UpdateSelection(const GridRole role)
{
	for (const auto& weakPtr : listeners.selection)
		if (const auto ptr = weakPtr.lock(); ptr)
			ptr->OnUpdate(role);
}

void SessionData::UpdateSelection()
{
	UpdateSelection(lastGridRole);
}

void SessionData::UpdateWeaponTransfer(const GridRole destinationRole, const int count)
{
	for (const auto& weakPtr : listeners.weaponTransfer)
		if (const auto ptr = weakPtr.lock(); ptr)
			ptr->OnUpdate(destinationRole, count);
}

void SessionData::UpdateInventorySorting(invbuilder::Weapon::Sorting sorting)
{
	this->sorting = std::move(sorting);

	for (const auto& weakPtr : listeners.inventorySorting)
		if (const auto ptr = weakPtr.lock(); ptr)
			ptr->OnUpdate(this->sorting);
}

void SessionData::OverrideWeapons(const WeaponContext::Vector& weapons)
{
	if (const auto& ptr = inventoryRetriever.lock(); ptr)
		ptr->Set(weapons);
	else
		assert(false && "no inventory retriever registered");
}

auto SessionData::GetInventory() const -> WeaponContext::WeakVector
{
	const auto& ptr = inventoryRetriever.lock();
	assert(ptr && "no inventory retriever registered");
	return ptr ? ptr->Get() : WeaponContext::WeakVector{};
}

auto SessionData::GetAttributes() const -> invbuilder::PlayerAttributes
{
	return {1.f * attributes.str, 1.f * attributes.dex, 1.f * attributes.int_, 1.f * attributes.fth, 1.f * attributes.lck};
}

auto SessionData::GetSorting() const -> const Sorting&
{
	return sorting;
}

auto SessionData::GetSelection() const -> const WeaponContext::WeakVector&
{
	return selection;
}

void SessionData::RegisterAttributesListener(const std::weak_ptr<IAttributesListener>& listener)
{
	listeners.attributes.push_back(listener);
}

void SessionData::RegisterInventorySortingListener(const std::weak_ptr<IInventorySortingListener>& listener)
{
	listeners.inventorySorting.push_back(listener);
}

void SessionData::RegisterSelectionListener(const std::weak_ptr<ISelectionListener>& listener)
{
	listeners.selection.push_back(listener);
}

void SessionData::RegisterWeaponTransferListener(const std::weak_ptr<IWeaponTransferListener>& listener)
{
	listeners.weaponTransfer.push_back(listener);
}

void SessionData::RegisterInventoryRetriever(std::weak_ptr<IInventoryRetriever> retriever)
{
	assert(inventoryRetriever.expired() && "only one inventory retriever may be registered");
	inventoryRetriever = std::move(retriever);
}
