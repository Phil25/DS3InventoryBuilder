#pragma once

#include <memory>

class IAttributesListener : public std::enable_shared_from_this<IAttributesListener>
{
protected:
	IAttributesListener() = default;

public:
	virtual ~IAttributesListener() = default;

	void Register();

	virtual void OnUpdate(const int str, const int dex, const int int_, const int fth, const int lck) = 0;
};
