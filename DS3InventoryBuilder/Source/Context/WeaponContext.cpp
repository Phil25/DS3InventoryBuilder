#include "WeaponContext.h"

#include <Database.h>
#include <AppMain.h>

namespace
{
	inline auto GetWeaponID(const std::string& name)
	{
		return wxGetApp().GetDatabase().GetWeapon(name).id;
	}

	inline auto GetWeaponInfusable(const std::string& name)
	{
		return wxGetApp().GetDatabase().GetWeapon(name).infusable;
	}

	inline auto GetWeaponUnique(const std::string& name)
	{
		return wxGetApp().GetDatabase().GetWeapon(name).unique;
	}
}

WeaponContext::WeaponContext(const int gridID, const int cardID, std::string name, const int level, const Infusion infusion, const RequirementsStatus requirementsStatus) noexcept
	: gridID(gridID), cardID(cardID), name(std::move(name))
	, id(GetWeaponID(this->name)), isUnique(GetWeaponUnique(this->name)), isInfusable(GetWeaponInfusable(this->name))
	, level(level), infusion(isInfusable ? infusion : Infusion::None), requirementsStatus(requirementsStatus)
{
}

WeaponContext::WeaponContext(std::string name, const int level, const int infusion) noexcept
	: gridID(0), cardID(0), name(std::move(name))
	, id(GetWeaponID(this->name)), isUnique(GetWeaponUnique(this->name)), isInfusable(GetWeaponInfusable(this->name))
	, level(level), infusion(isInfusable ? static_cast<Infusion>(infusion) : Infusion::None), requirementsStatus(RequirementsStatus::Met)
{
}

bool WeaponContext::IsValid() const noexcept
{
	const auto inf = static_cast<int>(infusion);
	return (0 <= level && level <= 10)
		&& (0 <= inf && inf < static_cast<int>(Infusion::Size))
		&& (isInfusable || (!isInfusable && inf == 0))
		&& id && !name.empty();
}

auto WeaponContext::GetCardID() const noexcept -> int
{
	return cardID;
}

void WeaponContext::SetCardID(const int cardID, const int useCount)
{
	// this should be done only after sorting, and before that selection should be emptied, removing the preview
	assert(useCount == 1 && "only this card should own the weapon context at this time");
	this->cardID = cardID;
}

auto WeaponContext::GetName() const noexcept -> const std::string&
{
	return name;
}

auto WeaponContext::GetID() const noexcept -> int
{
	return id;
}

bool WeaponContext::IsUnique() const noexcept
{
	return isUnique;
}

auto WeaponContext::GetLevel(const bool display) const noexcept -> int
{
	return display ? invbuilder::Database::GetDisplayLevel(isUnique, level) : level;
}

void WeaponContext::SetLevel(const int level) noexcept
{
	assert(0 <= level && level <= 10 && "illegal weapon level");
	this->level = level;
}

auto WeaponContext::GetInfusion() const noexcept -> Infusion
{
	return infusion;
}

void WeaponContext::SetInfusion(const Infusion infusion) noexcept
{
	this->infusion = isInfusable ? infusion : invbuilder::Weapon::Infusion::None;
}

auto WeaponContext::GetRequirementsStatus() const noexcept -> RequirementsStatus
{
	return requirementsStatus;
}

void WeaponContext::SetRequirementsStatus(const RequirementsStatus requirementsStatus) noexcept
{
	this->requirementsStatus = requirementsStatus;
}
