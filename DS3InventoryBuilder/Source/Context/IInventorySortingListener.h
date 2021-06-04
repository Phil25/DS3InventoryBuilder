#pragma once

#include <memory>
#include <Weapon.hpp>

class IInventorySortingListener : public std::enable_shared_from_this<IInventorySortingListener>
{
protected:
	IInventorySortingListener() = default;

public:
	virtual ~IInventorySortingListener() = default;

	void Register();

	virtual void OnUpdate(const invbuilder::Weapon::Sorting&) = 0;
};
