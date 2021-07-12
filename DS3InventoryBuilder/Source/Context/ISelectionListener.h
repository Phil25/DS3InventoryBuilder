#pragma once

#include <Utils/GridRole.hpp>
#include <memory>

class ISelectionListener : public std::enable_shared_from_this<ISelectionListener>
{
protected:
	ISelectionListener() = default;

public:
	virtual ~ISelectionListener() = default;

	void Register();

	virtual void OnUpdate(const GridRole role) = 0;
};
