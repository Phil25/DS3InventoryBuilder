#pragma once

#include <memory>
#include <Utils/GridRole.hpp>

class IWeaponTransferListener : public std::enable_shared_from_this<IWeaponTransferListener>
{
protected:
	IWeaponTransferListener() = default;

public:
	virtual ~IWeaponTransferListener() = default;

	void Register();

	virtual void OnUpdate(const GridRole destinationRole, const int count) = 0;
};
