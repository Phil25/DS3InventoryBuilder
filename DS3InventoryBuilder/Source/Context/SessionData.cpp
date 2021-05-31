#include "SessionData.h"

#include <Context/IAttributesListener.h>
#include <Context/ISelectionListener.h>
#include <cassert>

WeaponContext::WeaponContext(const int gridID, const int cardID, std::string name, const int level, const Infusion infusion) noexcept
	: gridID(gridID), cardID(cardID), name(std::move(name)), level(level), infusion(infusion)
{
}

auto WeaponContext::GetLevel() const noexcept -> int
{
	return level;
}

void WeaponContext::SetLevel(const int level) noexcept
{
	this->level = level;
}

auto WeaponContext::GetInfusion() const noexcept -> Infusion
{
	return infusion;
}

void WeaponContext::SetInfusion(const Infusion infusion) noexcept
{
	this->infusion = infusion;
}

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

	for (const auto& weakPtr : listeners.attributes)
		if (const auto ptr = weakPtr.lock(); ptr)
			ptr->OnUpdate(str, dex, int_, fth, lck);
}

void SessionData::UpdateSelection(std::vector<std::weak_ptr<WeaponContext>> selection)
{
	this->selection = std::move(selection);

	for (const auto& weakPtr : listeners.selection)
		if (const auto ptr = weakPtr.lock(); ptr)
			ptr->OnUpdate();
}

auto SessionData::GetSelection() const -> const SelectionVector&
{
	return selection;
}

auto SessionData::GetAttributes() const -> const invbuilder::PlayerAttributes&
{
	return {1.f * attributes.str, 1.f * attributes.dex, 1.f * attributes.int_, 1.f * attributes.fth, 1.f * attributes.lck};
}

void SessionData::RegisterAttributesListener(const std::weak_ptr<IAttributesListener>& listener)
{
	listeners.attributes.push_back(listener);
}

void SessionData::RegisterFinderOptionsListener(const std::weak_ptr<IFinderOptionsListener>& listener)
{
	listeners.finderOptions.push_back(listener);
}

void SessionData::RegisterFinderSortingListener(const std::weak_ptr<IFinderSortingListener>& listener)
{
	listeners.finderSorting.push_back(listener);
}

void SessionData::RegisterInventorySortingListener(const std::weak_ptr<IInventorySortingListener>& listener)
{
	listeners.inventorySorting.push_back(listener);
}

void SessionData::RegisterSelectionListener(const std::weak_ptr<ISelectionListener>& listener)
{
	listeners.selection.push_back(listener);
}
