#pragma once

#include <Panels/Title.h>
#include <memory>

class Preview final : public Title
{
	class AttributesListener;
	class SelectionListener;

public:
	class TextHeader;
	class WeaponLabel;
	class PreviewIcon;
	class WeaponSimple;
	class WeaponBook;

private:
	std::shared_ptr<AttributesListener> attributesListener;
	std::shared_ptr<SelectionListener> selectionListener;

	wxBoxSizer* sizer;

	WeaponLabel* label;
	PreviewIcon* icon;
	WeaponSimple* simple;
	WeaponBook* book;

public:
	Preview(wxWindow* parent);

	void SetShowSimple(const bool);
	void OnSelectionUpdate();
};