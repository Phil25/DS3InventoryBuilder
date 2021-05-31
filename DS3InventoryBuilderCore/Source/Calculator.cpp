#include "Calculator.h"

#include <algorithm>

namespace calc = invbuilder::calculator;

namespace
{
	struct Saturations final
	{
		// each attributes scale the following damage types
		struct Strength final { float physical; };
		struct Dexterity final { float physical; };
		struct Intelligence final { float magic, fire, dark; };
		struct Faith final { float physical, fire, lightning, dark; };
		struct Luck final { float physical, bleed, poison; };

		const Strength strength;
		const Dexterity dexterity;
		const Intelligence intelligence;
		const Faith faith;
		const Luck luck;
	};

	inline auto GetSaturations(const invbuilder::Database& db, const invbuilder::SaturationFunctionID& satIds,
		const invbuilder::PlayerAttributes& attribs, const bool twoHanding) -> Saturations
	{
		const auto actualStr = std::floor(attribs.strength + attribs.strength * .5f * twoHanding);
		const auto str = std::lround(std::min(actualStr, 99.f));
		const auto dex = std::lround(attribs.dexterity);
		const auto int_ = std::lround(attribs.intelligence);
		const auto fth = std::lround(attribs.faith);
		const auto lck = std::lround(attribs.luck);

		return {
			{
				db.GetSaturationFunction(satIds.physical)[str],
			},
			{
				db.GetSaturationFunction(satIds.physical)[dex],
			},
			{
				db.GetSaturationFunction(satIds.magic)[int_],
				db.GetSaturationFunction(satIds.fire)[int_],
				db.GetSaturationFunction(satIds.dark)[int_],
			},
			{
				db.GetSaturationFunction(satIds.physical)[fth],
				db.GetSaturationFunction(satIds.fire)[fth],
				db.GetSaturationFunction(satIds.lightning)[fth],
				db.GetSaturationFunction(satIds.dark)[fth],
			},
			{
				db.GetSaturationFunction(satIds.physical)[lck],
				db.GetSaturationFunction(satIds.bleed)[lck],
				db.GetSaturationFunction(satIds.poison)[lck],
			}
		};
	}
}

auto calc::AttackRating(const Database& db, const char* name, const Weapon::Infusion infusion, const int level,
	const PlayerAttributes& attribs, const bool twoHanding) -> std::pair<DamageTypes, Status>
{
	const auto& weapon = db.GetWeapon(name);

	const auto& satIds = weapon.properties.at(infusion).saturationFunctionID;
	const auto saturations = GetSaturations(db, satIds, attribs, twoHanding);

	const auto& scaling = weapon.properties.at(infusion).level[level].scaling;
	const auto luckCoefficient = weapon.luckCoefficient;
	const auto luckMod = weapon.properties.at(infusion).level[level].luckMod;

	auto damages = weapon.properties.at(infusion).level[level].damage;
	auto status = weapon.properties.at(infusion).level[level].status;

	damages.physical *= (1
		+ scaling.strength / 100 * saturations.strength.physical / 100
		+ scaling.dexterity / 100 * saturations.dexterity.physical / 100
		+ scaling.faith / 100 * saturations.faith.physical / 100 * (infusion == Weapon::Infusion::Blessed)
		+ scaling.luck / 100 * saturations.luck.physical / 100);

	damages.magic *= 1 + scaling.intelligence / 100 * saturations.intelligence.magic / 100;

	damages.fire *= (1
		+ scaling.intelligence / 100 * saturations.intelligence.fire / 100
		+ scaling.faith / 100 * saturations.faith.fire / 100);

	damages.lightning *= 1 + scaling.faith / 100 * saturations.faith.lightning / 100;

	damages.dark *= (1
		+ scaling.intelligence / 100 * saturations.intelligence.dark / 100
		+ scaling.faith / 100 * saturations.faith.dark / 100);

	status.bleed *= 1 + luckCoefficient / 100 * luckMod * saturations.luck.bleed / 100;
	status.poison *= 1 + luckCoefficient / 100 * luckMod * saturations.luck.poison / 100;
	// forst does not scale, use base damage

	return {std::move(damages), std::move(status)};
}
