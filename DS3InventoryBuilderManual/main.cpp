#include <iostream>
#include <Database.h>

constexpr char cr = '\n';

int main()
{
	using Infusion = invbuilder::Weapon::Infusion;
	using Type = invbuilder::Weapon::Type;

	const auto db = invbuilder::Database::Create();
	const auto& onyxBlade = db.GetWeapon("Onyx Blade");

	std::cout << onyxBlade.name << cr;
	std::cout << onyxBlade.infusable << cr;
	std::cout << onyxBlade.requirements.faith << cr;
	std::cout << onyxBlade.properties.at(Infusion::None).level[4].damage.dark << cr;
	std::cout << onyxBlade.properties.at(Infusion::None).level[2].absorption.lightning << cr;

	std::cout << (db.GetWeapon("Ringed Knight Paired Greatswords").type == Type::UltraGreatsword) << cr;
	std::cout << db.GetSaturationFunction(5)[33] << cr;

	std::cout << db.GetNames().size() << cr;

	return 0;
}