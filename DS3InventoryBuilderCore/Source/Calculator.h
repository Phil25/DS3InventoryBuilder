#pragma once

#include <Database.h>

namespace invbuilder::calculator
{
	auto AttackRating(const Database&, const char* weaponName, const Weapon::Infusion, const int weaponLevel,
		const PlayerAttributes&, const bool twoHanding=false) -> std::pair<DamageTypes, Status>;
}