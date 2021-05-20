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
		auto GetWeapon(const char* name) const -> const Weapon&;
		auto GetSaturationFunction(const size_t index) const -> const SaturationFunction&;

		static auto Create() -> Database;
	};
}