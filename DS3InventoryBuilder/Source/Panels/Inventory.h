#pragma once

#include <Panels/Title.h>
#include <Panels/WeaponGrid.h>

class Inventory final : public Title
{
	class WeaponTransferListener;
	std::shared_ptr<WeaponTransferListener> weaponTransferListener;

	WeaponGrid* grid;

public:
	Inventory(wxWindow* parent);
};