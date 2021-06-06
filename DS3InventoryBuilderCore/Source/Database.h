#pragma once

#include <unordered_map>
#include <Weapon.hpp>

namespace invbuilder
{
	class Database final
	{
		using WeaponNames = std::vector<std::string>;
		const WeaponNames names;

		using WeaponData = std::unordered_map<std::string_view, Weapon>;
		const WeaponData weapons;

		using SaturationFunction = std::array<float, 100>;
		using Saturations = std::vector<SaturationFunction>;
		const Saturations saturations;

		Database(WeaponNames names, WeaponData weapons, Saturations staurations);

	public:
		auto GetNames() const -> const WeaponNames&;
		auto GetWeapon(const size_t index) const -> const Weapon&;
		auto GetWeapon(const std::string& name) const -> const Weapon&;
		auto GetSaturationFunction(const size_t index) const -> const SaturationFunction&;
		auto GetImage(unsigned int& size, std::string& name) const -> uint8_t*;

		static auto Create() -> Database;

		static bool IsCastingTool(const Weapon&);
		static bool IsShield(const Weapon&);
		static bool IsRanged(const Weapon&);

		static auto ToString(const Weapon::Infusion) -> std::string;
		static auto ToString(const Weapon::Type) -> std::string;
		static auto ToString(const Weapon::Sorting::Method) -> std::string;
		static auto GetDisplayLevel(const bool isUnique, const int level) -> int;
		static auto GetScalingGrade(const float scaling) -> char;
	};
}