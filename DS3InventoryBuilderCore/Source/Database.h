#pragma once

#include <unordered_map>
#include <Weapon.hpp>

namespace invbuilder
{
	class Database final
	{
		using Weapons = std::unordered_map<std::string, Weapon>;
		const Weapons weapons;

		using SaturationFunction = std::array<float, 100>;
		using Saturations = std::vector<SaturationFunction>;
		const Saturations saturations;

		Database(Weapons weapons, Saturations staurations);

	public:
		auto GetWeapon(const char* name) const -> const Weapon&;
		auto GetSaturationFunction(size_t index) const -> const SaturationFunction&;

		static auto Create() -> Database;
	};
}