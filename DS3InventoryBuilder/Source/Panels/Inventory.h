#pragma once

#include <Panels/Title.h>
#include <Panels/WeaponGrid.h>

class Inventory final : public Title
{
	class WeaponTransferListener;
	class SelectionListener;

	std::shared_ptr<WeaponTransferListener> weaponTransferListener;
	std::shared_ptr<SelectionListener> selectionListener;

	WeaponGrid* grid;

public:
	Inventory(wxWindow* parent);
};