#pragma once

#include <Panels/Title.h>
#include <Panels/WeaponGrid.h>

class Finder final : public Title
{
	class AttributesListener;
	class SelectionListener;

	std::shared_ptr<AttributesListener> attributesListener;
	std::shared_ptr<SelectionListener> selectionListener;

	WeaponGrid* grid;

public:
	Finder(wxWindow* parent);
};