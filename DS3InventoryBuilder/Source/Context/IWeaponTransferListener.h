#pragma once

#include <memory>

class IWeaponTransferListener : public std::enable_shared_from_this<IWeaponTransferListener>
{
protected:
	IWeaponTransferListener() = default;

public:
	virtual ~IWeaponTransferListener() = default;

	void Register();

	virtual void OnUpdate(const int originGridID, const int count) = 0;
};
