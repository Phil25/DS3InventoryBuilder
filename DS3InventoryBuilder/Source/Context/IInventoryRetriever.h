#pragma once

#include <Context/WeaponContext.h>
#include <memory>

class IInventoryRetriever : public std::enable_shared_from_this<IInventoryRetriever>
{
protected:
	IInventoryRetriever() = default;

public:
	virtual ~IInventoryRetriever() = default;

	void Register();

	virtual auto Get() const -> WeaponContext::Vector = 0;
};
