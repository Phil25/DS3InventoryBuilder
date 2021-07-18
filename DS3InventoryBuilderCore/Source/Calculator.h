#pragma once

#include <Database.h>

namespace invbuilder::calculator
{
	auto AttackRating(const Database&, const char* weaponName, const Weapon::Infusion, const int weaponLevel,
		PlayerAttributes, const bool twoHanding=false) -> std::pair<DamageTypes, Status>;

	auto AttackRating(const Database&, const Weapon& weapon, const Weapon::Infusion, const int weaponLevel,
		PlayerAttributes, const bool twoHanding=false) -> std::pair<DamageTypes, Status>;
}