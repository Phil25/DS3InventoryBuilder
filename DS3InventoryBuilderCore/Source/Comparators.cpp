#include "Comparators.h"

#include <Calculator.h>

namespace comps = invbuilder::comparators;

#define RETURN_COMPARISON_ON_DIFFERENCE(a,b) \
	switch (Compare(a, b)) \
	{ \
	case -1: return true; \
	case 1: return false; \
	} \

namespace
{
	template <typename T>
	constexpr int Compare(const T& a, const T& b) noexcept
	{
		return (a < b) ? -1 : (a > b);
	}
}

bool comps::Default(const Weapon& weap1, const Weapon::Infusion inf1, const int lvl1, const Weapon& weap2, const Weapon::Infusion inf2, const int lvl2)
{
	RETURN_COMPARISON_ON_DIFFERENCE(weap1.orderID, weap2.orderID);
	RETURN_COMPARISON_ON_DIFFERENCE(inf1, inf2);
	RETURN_COMPARISON_ON_DIFFERENCE(lvl1, lvl2);
	return true; // weapons are the same
}

bool comps::Weight(const Weapon& weap1, const Weapon::Infusion inf1, const int lvl1, const Weapon& weap2, const Weapon::Infusion inf2, const int lvl2)
{
	RETURN_COMPARISON_ON_DIFFERENCE(weap1.weight, weap2.weight);
	return Default(weap1, inf1, lvl1, weap2, inf2, lvl2);
}

bool comps::AttackPower(const Weapon& weap1, const Weapon::Infusion inf1, const int lvl1, const Weapon& weap2, const Weapon::Infusion inf2, const int lvl2, const Database& db, const PlayerAttributes& attribs, const bool twoHanded)
{
	// TODO: this comparator is WRONG.
	// DS3 does really weird things when comparing weapons with very similar AR, and I can't figure out what
	// You can sometimes observe the order being ex. 62 > 61 > 62 even in-game, when checking out the value in the menu
	// However, if the difference is big enough (~2 AR), this works fine

	// DS3 always calculates two handed AR for bows/greatbows/crossbows
	const bool twoHanded1 = twoHanded || invbuilder::Database::IsRanged(weap1); 
	const bool twoHanded2 = twoHanded || invbuilder::Database::IsRanged(weap2);

	const auto& [damages1, _1] = calculator::AttackRating(db, weap1.name.c_str(), inf1, lvl1, attribs, twoHanded1);
	const auto& [damages2, _2] = calculator::AttackRating(db, weap2.name.c_str(), inf2, lvl2, attribs, twoHanded2);

	const auto ar1 = static_cast<int>(damages1.Total());
	const auto ar2 = static_cast<int>(damages2.Total());

	RETURN_COMPARISON_ON_DIFFERENCE(ar1, ar2);
	return Default(weap1, inf1, lvl1, weap2, inf2, lvl2);
}

bool comps::GuardAbsorption(const Weapon& weap1, const Weapon::Infusion inf1, const int lvl1, const Weapon& weap2, const Weapon::Infusion inf2, const int lvl2)
{
	const auto& abs1 = weap1.properties.at(inf1).level[lvl1].absorption;
	const auto& abs2 = weap2.properties.at(inf2).level[lvl2].absorption;

	RETURN_COMPARISON_ON_DIFFERENCE(abs1.Total(), abs2.Total());
	return Default(weap1, inf1, lvl1, weap2, inf2, lvl2);
}

bool comps::Effect(const Weapon& weap1, const Weapon::Infusion inf1, const int lvl1, const Weapon& weap2, const Weapon::Infusion inf2, const int lvl2)
{
	// the same as the default infusion order, but `none` is thrown at the end
	const auto order1 = inf1 == Weapon::Infusion::None ? 99 : static_cast<int>(inf1);
	const auto order2 = inf2 == Weapon::Infusion::None ? 99 : static_cast<int>(inf2);

	RETURN_COMPARISON_ON_DIFFERENCE(order1, order2);
	return Default(weap1, inf1, lvl1, weap2, inf2, lvl2);
}

bool comps::AttackPowerPrecise(const Weapon& weap1, const Weapon::Infusion inf1, const int lvl1, const Weapon& weap2, const Weapon::Infusion inf2, const int lvl2, const Database& db, const PlayerAttributes& attribs)
{
	const auto& [damages1, _1] = calculator::AttackRating(db, weap1.name.c_str(), inf1, lvl1, attribs);
	const auto& [damages2, _2] = calculator::AttackRating(db, weap2.name.c_str(), inf2, lvl2, attribs);

	RETURN_COMPARISON_ON_DIFFERENCE(damages1.Total(), damages2.Total());
	return Default(weap1, inf1, lvl1, weap2, inf2, lvl2);
}

bool comps::AttackPowerPreciseTwoHanded(const Weapon& weap1, const Weapon::Infusion inf1, const int lvl1, const Weapon& weap2, const Weapon::Infusion inf2, const int lvl2, const Database& db, const PlayerAttributes& attribs)
{
	const auto& [damages1, _1] = calculator::AttackRating(db, weap1.name.c_str(), inf1, lvl1, attribs, true);
	const auto& [damages2, _2] = calculator::AttackRating(db, weap2.name.c_str(), inf2, lvl2, attribs, true);

	RETURN_COMPARISON_ON_DIFFERENCE(damages1.Total(), damages2.Total());
	return Default(weap1, inf1, lvl1, weap2, inf2, lvl2);
}

bool comps::AttackPowerPreciseTwoHandedIfRequired(const Weapon& weap1, const Weapon::Infusion inf1, const int lvl1, const Weapon& weap2, const Weapon::Infusion inf2, const int lvl2, const Database& db, const PlayerAttributes& attribs)
{
	const bool twoHanded1 = attribs.strength * 1.5 < weap1.requirements.strength;
	const bool twoHanded2 = attribs.strength * 1.5 < weap2.requirements.strength;

	const auto& [damages1, _1] = calculator::AttackRating(db, weap1.name.c_str(), inf1, lvl1, attribs, twoHanded1);
	const auto& [damages2, _2] = calculator::AttackRating(db, weap2.name.c_str(), inf2, lvl2, attribs, twoHanded2);

	RETURN_COMPARISON_ON_DIFFERENCE(damages1.Total(), damages2.Total());
	return Default(weap1, inf1, lvl1, weap2, inf2, lvl2);
}

bool comps::Stability(const Weapon& weap1, const Weapon::Infusion inf1, const int lvl1, const Weapon& weap2, const Weapon::Infusion inf2, const int lvl2)
{
	const auto& stb1 = weap1.properties.at(inf1).level[lvl1].stability;
	const auto& stb2 = weap2.properties.at(inf2).level[lvl2].stability;

	RETURN_COMPARISON_ON_DIFFERENCE(stb1, stb2);
	return Default(weap1, inf1, lvl1, weap2, inf2, lvl2);
}

bool comps::StabilityThenGuardAbsorption(const Weapon& weap1, const Weapon::Infusion inf1, const int lvl1, const Weapon& weap2, const Weapon::Infusion inf2, const int lvl2)
{
	const auto& stb1 = weap1.properties.at(inf1).level[lvl1].stability;
	const auto& stb2 = weap2.properties.at(inf2).level[lvl2].stability;

	RETURN_COMPARISON_ON_DIFFERENCE(stb1, stb2);
	return GuardAbsorption(weap1, inf1, lvl1, weap2, inf2, lvl2);
}
