#pragma once

#include <memory>

class IAttributesListener : public std::enable_shared_from_this<IAttributesListener>
{
protected:
	IAttributesListener() = default;

public:
	virtual ~IAttributesListener() = default;

	void Register();

	virtual void OnUpdate() = 0;
};
