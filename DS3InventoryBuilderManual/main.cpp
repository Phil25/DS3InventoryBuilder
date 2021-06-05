#include <iostream>
#include <iomanip>
#include <Calculator.h>

constexpr char cr = '\n';

inline void PrintAttackRating(const invbuilder::DamageTypes& damage, const invbuilder::Status& status)
{
	float myOwnPhys = 502.f;
	std::cout << "Damage: ";
	std::cout << "Physical(" << std::setprecision(40) << damage.physical << "), ";
	std::cout << "Physical(" << static_cast<int>(damage.physical) << "), ";
	std::cout << "Physical(" << static_cast<int>(myOwnPhys) << "), ";
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
	const auto& [damage, status] = invbuilder::calculator::AttackRating(db, "Carthus Curved Greatsword", Infusion::None, 10, {40, 40, 11, 15, 7});

	PrintAttackRating(damage, status);

	return 0;
}