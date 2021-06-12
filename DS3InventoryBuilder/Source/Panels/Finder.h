#pragma once

#include <Panels/Title.h>
#include <Panels/WeaponGrid.h>
#include <memory>

class Finder final : public Title
{
	class AttributesListener;
	class SelectionListener;

	std::shared_ptr<AttributesListener> attributesListener;
	std::shared_ptr<SelectionListener> selectionListener;

	class FilterControls;
	FilterControls* controls;

	WeaponGrid* grid;
	invbuilder::Weapon::Sorting sorting{};

public:
	Finder(wxWindow* parent);

	void OnFilterControlsUpdate();
};