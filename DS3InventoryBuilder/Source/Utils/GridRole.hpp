#pragma once

enum class GridRole { None, Browser, Inventory };

inline constexpr auto OtherGrid(const GridRole role)
{
	using R = GridRole;
	return role == R::Browser ? R::Inventory : R::Browser;
}
