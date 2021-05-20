#pragma once

#include <string>
#include <array>
#include <map>
#include <PlayerAttributes.hpp>

namespace invbuilder
{
	struct SaturationFunctionID final
	{
		int physical, magic, fire, lightning, dark, bleed, poison;
	};

	struct LevelProperties final
	{
		struct Status final
		{
			float bleed, poison, frost;
		};

		struct DamageTypes final
		{
			float physical, magic, fire, lightning, dark;
		};

		struct Resistance final
		{
			float bleed, poison, frost, curse;
		};

		float luckMod;
		float stability;
		Status status;
		DamageTypes damage;
		PlayerAttributes scaling;
		DamageTypes absorption;
		Resistance resistance;
	};

	struct InfusionProperties final
	{
		SaturationFunctionID saturationFunctionID;
		std::array<LevelProperties, 11> level;
	};

	struct Weapon final
	{
		enum class Type
		{
			Dagger, StraightSword, Greatsword, UltraGreatsword, CurvedSword, CurvedGreatsword,
			ThrustingSword, Katana, Axe, Greataxe, Hammer, GreatHammer, Spear, Pike, Halberd,
			Reaper, Whip, Fist, Claw, Bow, Greatbow, Crossbow, Staff, PyromancyFlame, Talisman,
			SacredChime, Torch, SmallShield, Shield, Greatshield
		};

		enum class Infusion
		{
			None, Heavy, Sharp, Refined, Raw, Crystal, Simple, Fire, Chaos,
			Lightning, Blessed, Deep, Dark, Blood, Poison, Hollow
		};

		using Properties = std::map<Infusion, const InfusionProperties>;

		const std::string name;
		const Type type;
		const int orderID;
		const float luckCoefficient;

		const bool buffable;
		const bool infusable;
		const bool unique;

		const float weight;

		const PlayerAttributes requirements;
		const Properties properties;
	};
}
