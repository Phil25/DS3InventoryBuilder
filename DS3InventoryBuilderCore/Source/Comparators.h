#pragma once

#include <Database.h>

namespace invbuilder::comparators
{
	// in-game
	bool Default(const Weapon&, const Weapon::Infusion, const int level1, const Weapon&, const Weapon::Infusion, const int level2);
	bool Weight(const Weapon&, const Weapon::Infusion, const int level1, const Weapon&, const Weapon::Infusion, const int level2);
	bool AttackPower(const Weapon&, const Weapon::Infusion, const int level1, const Weapon&, const Weapon::Infusion, const int level2, const Database&, const PlayerAttributes&, const bool twoHanded=true);
	bool GuardAbsorption(const Weapon&, const Weapon::Infusion, const int level1, const Weapon&, const Weapon::Infusion, const int level2);
	bool Effect(const Weapon&, const Weapon::Infusion, const int level1, const Weapon&, const Weapon::Infusion, const int level2);

	// custom
	bool AttackPowerPrecise(const Weapon&, const Weapon::Infusion, const int level1, const Weapon&, const Weapon::Infusion, const int level2, const Database&, const PlayerAttributes&);
	bool AttackPowerPreciseTwoHanded(const Weapon&, const Weapon::Infusion, const int level1, const Weapon&, const Weapon::Infusion, const int level2, const Database&, const PlayerAttributes&);
	bool AttackPowerPreciseTwoHandedIfRequired(const Weapon&, const Weapon::Infusion, const int level1, const Weapon&, const Weapon::Infusion, const int level2, const Database&, const PlayerAttributes&);

	bool Stability(const Weapon&, const Weapon::Infusion, const int level1, const Weapon&, const Weapon::Infusion, const int level2);
	bool StabilityThenGuardAbsorption(const Weapon&, const Weapon::Infusion, const int level1, const Weapon&, const Weapon::Infusion, const int level2);
}