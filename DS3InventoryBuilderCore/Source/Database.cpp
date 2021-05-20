#include "Database.h"
#include <rapidjson/document.h>
#include <bzlib.h>
#include <vector>

using Database = invbuilder::Database;
using Weapon = invbuilder::Weapon;
using PlayerAttributes = invbuilder::PlayerAttributes;
using InfusionProperties = invbuilder::InfusionProperties;
using LevelProperties = invbuilder::LevelProperties;
using DamageTypes = invbuilder::DamageTypes;
using SaturationFunctionID = invbuilder::SaturationFunctionID;

namespace
{
	constexpr uint32_t maxDecompressedWeaponDataSize = 12'000'000; // bytes

	auto GetDecompressedWeaponData()
	{
		std::vector<uint8_t> compressed{
#include "Data/weapon_data.json.bz2.txt"
		};

		std::vector<char> decompressed;
		decompressed.reserve(maxDecompressedWeaponDataSize);
		unsigned int destLen = maxDecompressedWeaponDataSize;

		static_assert(sizeof(uint8_t) == sizeof(char));
		const auto source = reinterpret_cast<char*>(&compressed.front());
		const auto sourceLen = static_cast<unsigned int>(compressed.size());

		const auto success = BZ2_bzBuffToBuffDecompress(&decompressed.front(), &destLen, source, sourceLen, 0, 0);

		assert(success == 0 && "Failure in decompressing weapon data");
		(void)success;

		return decompressed;
	}

	auto ToWeaponType(const char* type)
	{
		switch (type[0])
		{
		case 'F': return Weapon::Type::Fist;
		case 'D': return Weapon::Type::Dagger;
		case 'S':
			switch (type[1])
			{
			case 't': 
				switch (type[2])
				{
					case 'r': return Weapon::Type::StraightSword;
					case 'a': return Weapon::Type::Staff;
					default: assert(false && "invalid type");
				}
			case 'w': return Weapon::Type::StraightSword; // Sword (Paired)
			case 'p':
				switch (type[3])
				{
				case 'a': return Weapon::Type::Spear;
				case 'c': return Weapon::Type::ThrustingSword; // Special Thrusting Sword
				default: assert(false && "invalid type");
				}
			case 'a': return Weapon::Type::SacredChime;
			case 'm': return Weapon::Type::SmallShield;
			case 'h': return Weapon::Type::Shield;
			default: assert(false && "invalid type");
			}
		case 'C':
			switch (type[1])
			{
			case 'u': 
				switch (type[7])
				{
				case 'S': return Weapon::Type::CurvedSword;
				case 'G': return Weapon::Type::CurvedGreatsword;
				default: assert(false && "invalid type");
				}
			case 'l': return Weapon::Type::Claw;
			case 'r': return Weapon::Type::Crossbow;
			default: assert(false && "invalid type");
			}
		case 'G': 
			switch (type[6])
			{
			case 'w': return Weapon::Type::Greatsword;
			case 'x': return Weapon::Type::Greataxe;
			case 'H': return Weapon::Type::GreatHammer;
			case 'h': return Weapon::Type::Greatshield;
			case 'o': return Weapon::Type::Greatbow;
			default: assert(false && "invalid type");
			}
		case 'T':
			switch (type[1])
			{
			case 'h': return Weapon::Type::ThrustingSword;
			case 'a': return Weapon::Type::Talisman;
			case 'o': return Weapon::Type::Torch;
			default: assert(false && "invalid type");
			}
		case 'K': return Weapon::Type::Katana;
		case 'U': return Weapon::Type::UltraGreatsword;
		case 'A': return Weapon::Type::Axe;
		case 'H':
			switch (type[2])
			{
			case 'l': return Weapon::Type::Halberd;
			case 'm': return Weapon::Type::Hammer;
			default: assert(false && "invalid type");
			}
		case 'P':
			switch (type[1])
			{
			case 'i': return Weapon::Type::Pike;
			case 'y': return Weapon::Type::PyromancyFlame;
			default: assert(false && "invalid type");
			}
		case 'R': return Weapon::Type::Reaper;
		case 'W': return Weapon::Type::Whip;
		case 'B': return Weapon::Type::Bow;
		default: assert(false && "invalid type");
		}

		assert(false && "invalid type");
		return Weapon::Type::Fist;
	}

	auto ToWeaponInfusion(const char* infusion)
	{
		switch (infusion[0])
		{
		case '-': return Weapon::Infusion::None;
		case 'H':
			switch (infusion[1])
			{
			case 'e': return Weapon::Infusion::Heavy;
			case 'o': return Weapon::Infusion::Hollow;
			default: assert(false && "invalid infusion");
			}
		case 'S': return Weapon::Infusion::Sharp;
		case 'R':
			switch (infusion[1])
			{
			case 'e': return Weapon::Infusion::Refined;
			case 'a': return Weapon::Infusion::Raw;
			default: assert(false && "invalid infusion");
			}
		case 'C':
			switch (infusion[1])
			{
			case 'r': return Weapon::Infusion::Crystal;
			case 'h': return Weapon::Infusion::Chaos;
			default: assert(false && "invalid infusion");
			}
		case 'F': return Weapon::Infusion::Fire;
		case 'L': return Weapon::Infusion::Lightning;
		case 'B':
			switch (infusion[2])
			{
			case 'e': return Weapon::Infusion::Blessed;
			case 'o': return Weapon::Infusion::Blood;
			default: assert(false && "invalid infusion");
			}
		case 'D':
			switch (infusion[1])
			{
			case 'e': return Weapon::Infusion::Deep;
			case 'a': return Weapon::Infusion::Dark;
			default: assert(false && "invalid infusion");
			}
		case 'P': return Weapon::Infusion::Poison;
		default: assert(false && "invalid infusion");
		}

		assert(false && "invalid infusion");
		return Weapon::Infusion::None;
	}

	template <bool ExpectLuck>
	auto ParsePlayerAttributes(const rapidjson::Value& attribs) -> PlayerAttributes
	{
		float lck = 0.f;
		if constexpr (ExpectLuck)
			lck = attribs["lck"].GetFloat();

		return {
			attribs["str"].GetFloat(),
			attribs["dex"].GetFloat(),
			attribs["int"].GetFloat(),
			attribs["fth"].GetFloat(),
			lck
		};
	}

	auto ParseDamageTypes(const rapidjson::Value& types) -> DamageTypes
	{
		return {
			types["physical"].GetFloat(),
			types["magic"].GetFloat(),
			types["fire"].GetFloat(),
			types["lightning"].GetFloat(),
			types["dark"].GetFloat(),
		};
	}

	auto ParseSaturationIndices(const rapidjson::Value& ids) -> SaturationFunctionID
	{
		return {
			ids["physical"].GetInt(),
			ids["magic"].GetInt(),
			ids["fire"].GetInt(),
			ids["lightning"].GetInt(),
			ids["dark"].GetInt(),
			ids["bleed"].GetInt(),
			ids["poison"].GetInt(),
		};
	}

	auto ParseLevelProperties(const rapidjson::Value& level) -> LevelProperties
	{
		return {
			level["lck_mod"].GetFloat(),
			level["stability"].GetFloat(),
			{
				level["status"]["bleed"].GetFloat(),
				level["status"]["poison"].GetFloat(),
				level["status"]["frost"].GetFloat(),
			},
			ParseDamageTypes(level["damage"]),
			ParsePlayerAttributes<true>(level["scaling"]),
			ParseDamageTypes(level["absorption"]),
			{
				level["resistance"]["bleed"].GetFloat(),
				level["resistance"]["poison"].GetFloat(),
				level["resistance"]["frost"].GetFloat(),
				level["resistance"]["curse"].GetFloat(),
			},
		};
	}

	auto ParseInfusion(const rapidjson::Value& infusion) -> InfusionProperties
	{
		return {
			ParseSaturationIndices(infusion["saturation_index"]),
			{{
				ParseLevelProperties(infusion["level"][0]),
				ParseLevelProperties(infusion["level"][1]),
				ParseLevelProperties(infusion["level"][2]),
				ParseLevelProperties(infusion["level"][3]),
				ParseLevelProperties(infusion["level"][4]),
				ParseLevelProperties(infusion["level"][5]),
				ParseLevelProperties(infusion["level"][6]),
				ParseLevelProperties(infusion["level"][7]),
				ParseLevelProperties(infusion["level"][8]),
				ParseLevelProperties(infusion["level"][9]),
				ParseLevelProperties(infusion["level"][10]),
			}}
		};
	}

	auto ParseProperties(const rapidjson::Value& infusions) -> Weapon::Properties
	{
		Weapon::Properties properties;

		for (auto it = infusions.MemberBegin(); it != infusions.MemberEnd(); ++it)
		{
			properties.emplace(ToWeaponInfusion(it->name.GetString()), ParseInfusion(it->value));
		}

		return properties;
	}

	auto ParseWeapon(const rapidjson::Value& data) -> Weapon
	{
		return {
			data["name"].GetString(),
			ToWeaponType(data["class"].GetString()),
			data["order"].GetInt(),
			data["lck_coeff"].GetFloat(),
			data["buffable"].GetBool(),
			data["infusable"].GetBool(),
			data["unique"].GetBool(),
			data["weight"].GetFloat(),
			ParsePlayerAttributes<false>(data["requirements"]),
			ParseProperties(data["infusion"])
		};
	}
}

Database::Database(WeaponNames names, WeaponData weapons, Saturations saturations)
	: names(std::move(names))
	, weapons(std::move(weapons))
	, saturations(std::move(saturations))
{
}

auto Database::GetNames() const -> const WeaponNames&
{
	return names;
}

auto Database::GetWeapon(const size_t index) const -> const Weapon&
{
	assert(index < names.size() && "invalid weapon index");
	return weapons.at(names[index]);
}

auto Database::GetWeapon(const char* name) const -> const Weapon&
{
	return weapons.at(name);
}

auto Database::GetSaturationFunction(const size_t index) const -> const SaturationFunction&
{
	assert(index < saturations.size() && "invalid saturation function index");
	return saturations[index];
}

auto Database::Create() -> Database
{
	const auto weaponData = GetDecompressedWeaponData();

	rapidjson::Document doc;
	doc.Parse(weaponData.data());

	WeaponNames names;
	WeaponData weapons;
	names.reserve(286);
	weapons.reserve(286);

	for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it)
	{
		const auto& ref = names.emplace_back(it->name.GetString());
		weapons.emplace(ref, ParseWeapon(it->value));
	}

	Saturations saturations{
#include "Data/saturations.txt"
	};

	assert(names.size() == weapons.size() && "names and weapons container mismatch");
	assert(names.size() && "empty weapon data");
	assert(names.size() == 286 && "there are 286 weapons in the game");
	return Database(std::move(names), std::move(weapons), std::move(saturations));
}
