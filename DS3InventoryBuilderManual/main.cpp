#include <iostream>
#include <Calculator.h>

constexpr char cr = '\n';

inline void PrintAttackRating(const invbuilder::DamageTypes& damage, const invbuilder::Status& status)
{
	std::cout << "Damage: ";
	std::cout << "Physical(" << damage.physical << "), ";
	std::cout << "Magic(" << damage.magic << "), ";
	std::cout << "Fire(" << damage.fire << "), ";
	std::cout << "Lightning(" << damage.lightning << "), ";
	std::cout << "Dark(" << damage.dark << ')' << cr;

	std::cout << "Total : " << damage.Total() << " AR" << cr;

	std::cout << "Status: ";
	std::cout << "Bleed(" << status.bleed << "), ";
	std::cout << "Poison(" << status.poison << "), ";
	std::cout << "Frost(" << status.frost << ')' << cr;
}

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

	const auto& [damage, status] = invbuilder::calculator::AttackRating(db, "Drakeblood Greatsword", Infusion::Poison, 7, {40, 35, 15, 25, 30}, true);
	std::cout << "=== Poison Drakeblood Greatsword +7:" << cr;
	PrintAttackRating(damage, status);

	return 0;
}