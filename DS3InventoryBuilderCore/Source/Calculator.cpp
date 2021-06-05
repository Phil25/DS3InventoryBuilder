#include "Calculator.h"

#include <algorithm>
#include <bitset>

namespace calc = invbuilder::calculator;

namespace
{
	inline auto HasElementalDamage(const invbuilder::DamageTypes& types)
	{
		return types.magic || types.fire || types.lightning || types.dark;
	}

	inline void RoundFloat(float& val)
	{
		val = std::round(val * 10000.f) / 10000.f;
	}

	namespace flags
	{
		enum Flags
		{
			StrDexScaleElemental = 0,
			IntScalesPhysical,
			FthScalesPhysical,
			FthScalesMagic,
			FthPenalizesPhysical,
			FthPenalizesMagic,
			LckScalesPhysical,
			LckPhysicalMultiplierBonus,
			Size,
		};

		inline bool GetStrDexScaleElemental(const invbuilder::Weapon& weapon)
		{
			return weapon.id == 16040000 // Dancer's Enchanted Swords 
				|| weapon.id == 4130000; // Demon's Scar
		}

		inline bool GetIntScalesPhysical(const invbuilder::Weapon& weapon)
		{
			return weapon.id == 14130000; // Darkmoon Longbow
		}

		inline bool GetFthScalesPhysical(const invbuilder::Weapon& weapon, const invbuilder::Weapon::Infusion infusion, const invbuilder::DamageTypes& damages)
		{
			return infusion == invbuilder::Weapon::Infusion::Blessed
				|| weapon.id == 10180000 // Friede's Scythe
				|| weapon.id == 9260000 // Crucifix of the Mad King
				|| weapon.id == 20310000 // Dragonhead Shield
				|| weapon.id == 9180000 // Saint's Bident
				|| (!HasElementalDamage(damages) && !invbuilder::Database::IsCastingTool(weapon) && !invbuilder::Database::IsShield(weapon));
		}

		inline bool GetFthScalesMagic(const invbuilder::Weapon& weapon)
		{
			return weapon.id == 13120000; // Golden Ritual Spear
		}

		inline bool GetFthPenalizesPhysical(const invbuilder::Weapon& weapon, const invbuilder::Weapon::Infusion infusion)
		{
			return infusion == invbuilder::Weapon::Infusion::Blessed
				|| (weapon.id != 2200000 && weapon.id != 10070000); // Astora Straight Sword & Pontiff Knight Great Scythe
		}

		inline bool GetFthPenalizesMagic(const invbuilder::Weapon& weapon)
		{
			return weapon.id == 13120000; // Golden Ritual Spear
		}

		inline bool GetLckScalesPhysical(const invbuilder::Weapon& weapon)
		{
			return !invbuilder::Database::IsCastingTool(weapon);
		}

		inline bool GetLckPhysicalMultiplierBonus(const invbuilder::Weapon& weapon)
		{
			return weapon.id == 2200000; // Astora Straight Sword
		}

		using Bits = std::bitset<Flags::Size>;

		inline auto Get(const invbuilder::Weapon& weapon, const invbuilder::Weapon::Infusion infusion, const invbuilder::DamageTypes& damages)
		{
			Bits flags;
			flags.set(flags::StrDexScaleElemental, GetStrDexScaleElemental(weapon));
			flags.set(flags::IntScalesPhysical, GetIntScalesPhysical(weapon));
			flags.set(flags::FthScalesPhysical, GetFthScalesPhysical(weapon, infusion, damages));
			flags.set(flags::FthScalesMagic, GetFthScalesMagic(weapon));
			flags.set(flags::FthPenalizesPhysical, GetFthPenalizesPhysical(weapon, infusion));
			flags.set(flags::FthPenalizesMagic, GetFthPenalizesMagic(weapon));
			flags.set(flags::LckScalesPhysical, GetLckScalesPhysical(weapon));
			flags.set(flags::LckPhysicalMultiplierBonus, GetLckPhysicalMultiplierBonus(weapon));
			return flags;
		}
	}

	struct Saturations final
	{
		// each attribute scales the following damage types
		struct Strength final { float physical, magic, fire; };
		struct Dexterity final { float physical, magic, fire; };
		struct Intelligence final { float physical, magic, fire, dark; };
		struct Faith final { float physical, magic, fire, lightning, dark; };
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
				db.GetSaturationFunction(satIds.physical)[int_],
				db.GetSaturationFunction(satIds.magic)[int_],
				db.GetSaturationFunction(satIds.fire)[int_],
				db.GetSaturationFunction(satIds.dark)[int_],
			},
			{
				db.GetSaturationFunction(satIds.physical)[fth],
				db.GetSaturationFunction(satIds.magic)[fth],
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

	inline auto IsPhysicalPenalized(const invbuilder::PlayerAttributes& attribs, const invbuilder::PlayerAttributes& requirements, const invbuilder::PlayerAttributes& scaling, const flags::Bits flags)
	{
		return (attribs.strength < requirements.strength)
			|| (attribs.dexterity < requirements.dexterity)
			|| (attribs.intelligence < requirements.intelligence && (flags.test(flags::IntScalesPhysical)))
			|| (attribs.faith < requirements.faith && (flags.test(flags::FthPenalizesPhysical)) && (flags.test(flags::FthScalesPhysical)));
	}

	inline auto IsMagicPenalized(const invbuilder::PlayerAttributes& attribs, const invbuilder::PlayerAttributes& requirements, const invbuilder::PlayerAttributes& scaling, const flags::Bits flags)
	{
		return (attribs.strength < requirements.strength&& scaling.strength > 0.f && (flags.test(flags::StrDexScaleElemental)))
			|| (attribs.dexterity < requirements.dexterity&& scaling.dexterity > 0.f && (flags.test(flags::StrDexScaleElemental)))
			|| (attribs.intelligence < requirements.intelligence&& scaling.intelligence > 0.f)
			|| (attribs.faith < requirements.faith && scaling.faith > 0.f && (flags.test(flags::FthPenalizesMagic)));
	}

	inline auto IsFirePenalized(const invbuilder::PlayerAttributes& attribs, const invbuilder::PlayerAttributes& requirements, const invbuilder::PlayerAttributes& scaling, const flags::Bits flags)
	{
		return (attribs.strength < requirements.strength && scaling.strength > 0.f && (flags.test(flags::StrDexScaleElemental)))
			|| (attribs.dexterity < requirements.dexterity && scaling.dexterity > 0.f && (flags.test(flags::StrDexScaleElemental)))
			|| (attribs.intelligence < requirements.intelligence)
			|| (attribs.faith < requirements.faith);
	}

	inline auto IsLightningPenalized(const invbuilder::PlayerAttributes& attribs, const invbuilder::PlayerAttributes& requirements, const invbuilder::PlayerAttributes& scaling)
	{
		return attribs.faith < requirements.faith;
	}

	inline auto IsDarkPenalized(const invbuilder::PlayerAttributes& attribs, const invbuilder::PlayerAttributes& requirements, const invbuilder::PlayerAttributes& scaling)
	{
		return (attribs.intelligence < requirements.intelligence)
			|| (attribs.faith < requirements.faith);
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

	const auto flags = flags::Get(weapon, infusion, damages);
	const auto luckMultiplierBonus = 1 + (1.35f * (flags.test(flags::LckPhysicalMultiplierBonus)));

	damages.physical *= IsPhysicalPenalized(attribs, requirements, scaling, flags)
		? 0.6f
		: 1.f
			+ scaling.strength / 100 * saturations.strength.physical / 100
			+ scaling.dexterity / 100 * saturations.dexterity.physical / 100
			+ scaling.intelligence / 100 * saturations.intelligence.physical / 100 * (flags.test(flags::IntScalesPhysical))
			+ scaling.faith / 100 * saturations.faith.physical / 100 * (flags.test(flags::FthScalesPhysical))
			+ scaling.luck / 100 * saturations.luck.physical / 100 * (flags.test(flags::LckScalesPhysical));

	damages.magic *= IsMagicPenalized(attribs, requirements, scaling, flags)
		? 0.6f
		: 1.f
			+ scaling.intelligence / 100 * saturations.intelligence.magic / 100
			+ scaling.strength / 100 * saturations.strength.magic / 100 * (flags.test(flags::StrDexScaleElemental))
			+ scaling.dexterity / 100 * saturations.dexterity.magic / 100 * (flags.test(flags::StrDexScaleElemental))
			+ scaling.faith / 100 * saturations.faith.magic / 100 * (flags.test(flags::FthScalesMagic));

	damages.fire *= IsFirePenalized(attribs, requirements, scaling, flags)
		? 0.6f
		: 1.f
			+ scaling.intelligence / 100 * saturations.intelligence.fire / 100
			+ scaling.faith / 100 * saturations.faith.fire / 100
			+ scaling.strength / 100 * saturations.strength.fire / 100 * (flags.test(flags::StrDexScaleElemental))
			+ scaling.dexterity / 100 * saturations.dexterity.fire / 100 * (flags.test(flags::StrDexScaleElemental));

	damages.lightning *= IsLightningPenalized(attribs, requirements, scaling)
		? 0.6f
		: 1.f + scaling.faith / 100 * saturations.faith.lightning / 100;

	damages.dark *= IsDarkPenalized(attribs, requirements, scaling)
		? 0.6f
		: 1.f
			+ scaling.intelligence / 100 * saturations.intelligence.dark / 100
			+ scaling.faith / 100 * saturations.faith.dark / 100;

	status.bleed *= 1 + luckCoefficient / 100 * luckMod * saturations.luck.bleed / 100 * luckMultiplierBonus;
	status.poison *= 1 + luckCoefficient / 100 * luckMod * saturations.luck.poison / 100 * luckMultiplierBonus;
	// frost does not scale, use base damage

	RoundFloat(damages.physical);
	RoundFloat(damages.magic);
	RoundFloat(damages.fire);
	RoundFloat(damages.lightning);
	RoundFloat(damages.dark);
	RoundFloat(status.bleed);
	RoundFloat(status.poison);

	return {std::move(damages), std::move(status)};
}
