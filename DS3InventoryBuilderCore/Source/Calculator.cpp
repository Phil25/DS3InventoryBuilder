#include "Calculator.h"

#include <algorithm>

namespace calc = invbuilder::calculator;

namespace
{
	struct Saturations final
	{
		// each attribute scales the following damage types
		struct Strength final { float physical, magic, fire; };
		struct Dexterity final { float physical, magic, fire; };
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
				db.GetSaturationFunction(satIds.magic)[str],
				db.GetSaturationFunction(satIds.fire)[str],
			},
			{
				db.GetSaturationFunction(satIds.physical)[dex],
				db.GetSaturationFunction(satIds.magic)[dex],
				db.GetSaturationFunction(satIds.fire)[dex],
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

	inline auto HasElementalDamage(const invbuilder::DamageTypes& types)
	{
		return types.magic || types.fire || types.lightning || types.dark;
	}

	inline auto IsFaithScalingException(const int id)
	{
		return id == 10180000 // Friede's Scythe
			|| id == 9260000; // Crucifix of the Mad King
	}

	inline auto IsPhysicalPenalized(const invbuilder::PlayerAttributes& attribs, const invbuilder::PlayerAttributes& requirements, const invbuilder::PlayerAttributes& scaling, const bool faithScalesPhysical)
	{
		return (attribs.strength < requirements.strength)
			|| (attribs.dexterity < requirements.dexterity)
			|| (attribs.faith < requirements.faith && scaling.faith > 0.f && faithScalesPhysical);
	}

	inline auto IsMagicPenalized(const invbuilder::PlayerAttributes& attribs, const invbuilder::PlayerAttributes& requirements, const invbuilder::PlayerAttributes& scaling, const bool strScalesElemental, const bool dexScalesElemental, const bool fthLackPenalizesMagic)
	{
		return (attribs.strength < requirements.strength && scaling.strength > 0.f && strScalesElemental)
			|| (attribs.dexterity < requirements.dexterity && scaling.dexterity > 0.f && dexScalesElemental)
			|| (attribs.intelligence < requirements.intelligence && scaling.intelligence > 0.f)
			|| (attribs.faith < requirements.faith && scaling.faith > 0.f && fthLackPenalizesMagic);
	}

	inline auto IsFirePenalized(const invbuilder::PlayerAttributes& attribs, const invbuilder::PlayerAttributes& requirements, const invbuilder::PlayerAttributes& scaling, const bool strScalesElemental, const bool dexScalesElemental)
	{
		return (attribs.strength < requirements.strength && scaling.strength > 0.f && strScalesElemental)
			|| (attribs.dexterity < requirements.dexterity && scaling.dexterity > 0.f && dexScalesElemental)
			|| (attribs.intelligence < requirements.intelligence && scaling.intelligence > 0.f)
			|| (attribs.faith < requirements.faith && scaling.faith > 0.f);
	}

	inline auto IsLightningPenalized(const invbuilder::PlayerAttributes& attribs, const invbuilder::PlayerAttributes& requirements, const invbuilder::PlayerAttributes& scaling)
	{
		return attribs.faith < requirements.faith && scaling.faith > 0.f;
	}

	inline auto IsDarkPenalized(const invbuilder::PlayerAttributes& attribs, const invbuilder::PlayerAttributes& requirements, const invbuilder::PlayerAttributes& scaling)
	{
		return (attribs.intelligence < requirements.intelligence && scaling.intelligence > 0.f)
			|| (attribs.faith < requirements.faith && scaling.faith > 0.f);
	}
}

auto calc::AttackRating(const Database& db, const char* name, const Weapon::Infusion infusion, const int level,
	const PlayerAttributes& attribs, const bool twoHanding) -> std::pair<DamageTypes, Status>
{
	const auto& weapon = db.GetWeapon(name);

	const auto& satIds = weapon.properties.at(infusion).saturationFunctionID;
	const auto saturations = GetSaturations(db, satIds, attribs, twoHanding);

	const auto& requirements = weapon.requirements;
	const auto& scaling = weapon.properties.at(infusion).level[level].scaling;
	const auto luckCoefficient = weapon.luckCoefficient;
	const auto luckMod = weapon.properties.at(infusion).level[level].luckMod;

	auto damages = weapon.properties.at(infusion).level[level].damage;
	auto status = weapon.properties.at(infusion).level[level].status;

	const bool fthScalesPhysical = (infusion == Weapon::Infusion::Blessed || !HasElementalDamage(damages) || IsFaithScalingException(weapon.id)) && !Database::IsCastingTool(weapon);
	const bool lckScalesPhysical = !Database::IsCastingTool(weapon);
	const bool strScalesElemental = weapon.id == 16040000; // Dancer's Enchanted Swords
	const bool dexScalesElemental = weapon.id == 16040000; // Dancer's Enchanted Swords
	const bool fthLackPenalizesMagic = weapon.id == 13120000; // Golden Ritual Spear

	damages.physical *= IsPhysicalPenalized(attribs, requirements, scaling, fthScalesPhysical)
		? 0.6f
		: 1.f
			+ scaling.strength / 100 * saturations.strength.physical / 100
			+ scaling.dexterity / 100 * saturations.dexterity.physical / 100
			+ scaling.faith / 100 * saturations.faith.physical / 100 * fthScalesPhysical
			+ scaling.luck / 100 * saturations.luck.physical / 100 * lckScalesPhysical;

	damages.magic *= IsMagicPenalized(attribs, requirements, scaling, strScalesElemental, dexScalesElemental, fthLackPenalizesMagic)
		? 0.6f
		: 1.f
			+ scaling.intelligence / 100 * saturations.intelligence.magic / 100
			+ scaling.strength / 100 * saturations.strength.magic / 100 * strScalesElemental
			+ scaling.dexterity / 100 * saturations.dexterity.magic / 100 * dexScalesElemental;

	damages.fire *= IsFirePenalized(attribs, requirements, scaling, strScalesElemental, dexScalesElemental)
		? 0.6f
		: 1.f
			+ scaling.intelligence / 100 * saturations.intelligence.fire / 100
			+ scaling.faith / 100 * saturations.faith.fire / 100
			+ scaling.strength / 100 * saturations.strength.fire / 100 * strScalesElemental
			+ scaling.dexterity / 100 * saturations.dexterity.fire / 100 * dexScalesElemental;

	damages.lightning *= IsLightningPenalized(attribs, requirements, scaling)
		? 0.6f
		: 1.f + scaling.faith / 100 * saturations.faith.lightning / 100;

	damages.dark *= IsDarkPenalized(attribs, requirements, scaling)
		? 0.6f
		: 1.f
			+ scaling.intelligence / 100 * saturations.intelligence.dark / 100
			+ scaling.faith / 100 * saturations.faith.dark / 100;

	status.bleed *= 1 + luckCoefficient / 100 * luckMod * saturations.luck.bleed / 100;
	status.poison *= 1 + luckCoefficient / 100 * luckMod * saturations.luck.poison / 100;
	// frost does not scale, use base damage

	return {std::move(damages), std::move(status)};
}
