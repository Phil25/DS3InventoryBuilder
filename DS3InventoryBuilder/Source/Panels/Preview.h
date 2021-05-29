#pragma once

#include <Panels/Title.h>

class Preview final : public Title
{
	class AttributesListener;
	class SelectionListener;

	std::shared_ptr<AttributesListener> attributesListener;
	std::shared_ptr<SelectionListener> selectionListener;

	wxStaticText* label;

public:
	Preview(wxWindow* parent);

	void OnSelectionUpdate();
};