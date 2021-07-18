#include "WeaponContext.h"

#include <Database.h>
#include <AppMain.h>

namespace
{
	inline auto GetWeaponProperties(const std::string& name) -> WeaponContext::Properties
	{
		const auto& weapon = wxGetApp().GetDatabase().GetWeapon(name);
		return {weapon.id, weapon.unique, weapon.infusable, weapon.type};
	}
}

WeaponContext::WeaponContext(const int cardID, std::string name, const int level, const Infusion infusion, const RequirementsStatus requirementsStatus) noexcept
	: cardID(cardID), name(std::move(name)), properties(GetWeaponProperties(this->name))
	, level(level), infusion(properties.infusable ? infusion : Infusion::None), requirementsStatus(requirementsStatus)
{
}

WeaponContext::WeaponContext(std::string name, const int level, const int infusion) noexcept
	: cardID(cardID), name(std::move(name)), properties(GetWeaponProperties(this->name))
	, level(level), infusion(properties.infusable ? static_cast<Infusion>(infusion) : Infusion::None), requirementsStatus(RequirementsStatus::Met)
{
}

bool WeaponContext::IsValid() const noexcept
{
	const auto inf = static_cast<int>(infusion);
	return (0 <= level && level <= 10)
		&& (0 <= inf && inf < static_cast<int>(Infusion::Size))
		&& !name.empty() && properties.id > 0
		&& (properties.infusable || (!properties.infusable && inf == 0));
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
	return properties.id;
}

bool WeaponContext::IsUnique() const noexcept
{
	return properties.unique;
}

auto WeaponContext::GetType() const noexcept -> Weapon::Type
{
	return properties.type;
}

auto WeaponContext::GetLevel(const bool display) const noexcept -> int
{
	return display ? invbuilder::Database::GetDisplayLevel(properties.unique, level) : level;
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
	this->infusion = properties.infusable ? infusion : invbuilder::Weapon::Infusion::None;
}

auto WeaponContext::GetRequirementsStatus() const noexcept -> RequirementsStatus
{
	return requirementsStatus;
}

void WeaponContext::SetRequirementsStatus(const RequirementsStatus requirementsStatus) noexcept
{
	this->requirementsStatus = requirementsStatus;
}
