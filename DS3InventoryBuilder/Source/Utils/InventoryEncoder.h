#pragma once

#include <Context/WeaponContext.h>

namespace inventory_encoder
{
	auto Encode(const WeaponContext::WeakVector&) -> std::string;
	auto Decode(std::string inventoryCode) -> WeaponContext::Vector;

	enum class Exception { Empty, Invalid, RevisionNotSupported };
};
