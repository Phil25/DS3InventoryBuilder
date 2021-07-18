#pragma once

#include <vector>
#include <memory>
#include <Weapon.hpp>

class WeaponContext final
{
public:
	using WeakVector = std::vector<std::weak_ptr<WeaponContext>>;
	using Vector = std::vector<std::shared_ptr<WeaponContext>>;

	enum class RequirementsStatus
	{
		NotMet, TwoHanded, Met
	};

	struct Properties final
	{
		const int id;
		const bool unique;
		const bool infusable;
	};

private:
	using Infusion = invbuilder::Weapon::Infusion;

	int cardID{};

	const std::string name;
	const Properties properties;

	int level;
	Infusion infusion;
	RequirementsStatus requirementsStatus;

public:
	WeaponContext(const int cardID, std::string name, const int level=10, const Infusion=Infusion::None, const RequirementsStatus=RequirementsStatus::Met) noexcept;
	WeaponContext(std::string name, const int level, const int infusion) noexcept;

	bool IsValid() const noexcept;

	auto GetCardID() const noexcept -> int;
	void SetCardID(const int cardID, const int useCount);

	auto GetName() const noexcept -> const std::string&;
	auto GetID() const noexcept -> int;
	bool IsUnique() const noexcept;

	auto GetLevel(const bool display=false) const noexcept -> int;
	void SetLevel(const int level) noexcept;

	auto GetInfusion() const noexcept -> Infusion;
	void SetInfusion(const Infusion infusion) noexcept;

	auto GetRequirementsStatus() const noexcept -> RequirementsStatus;
	void SetRequirementsStatus(const RequirementsStatus) noexcept;
};
