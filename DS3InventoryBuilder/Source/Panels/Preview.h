#pragma once

#include <Panels/Title.h>

class Preview final : public Title
{
	class AttributesListener;
	class SelectionListener;

public:
	class TextHeader;
	class PreviewIcon;
	class WeaponBook;

private:
	std::shared_ptr<AttributesListener> attributesListener;
	std::shared_ptr<SelectionListener> selectionListener;

	TextHeader* label;
	PreviewIcon* icon;
	WeaponBook* book;

public:
	Preview(wxWindow* parent);

	void OnSelectionUpdate();
};