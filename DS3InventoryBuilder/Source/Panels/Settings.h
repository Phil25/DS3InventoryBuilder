#pragma once

#include <Panels/Title.h>

class Settings final : public Title
{
	class Attribute;
	Attribute* str{nullptr};
	Attribute* dex{nullptr};
	Attribute* int_{nullptr};
	Attribute* fth{nullptr};
	Attribute* lck{nullptr};

	class InventorySorting;
	InventorySorting* inventorySorting;

	class MenuBar;
	MenuBar* menuBar;

public:
	Settings(wxWindow* parent);

private:
	void CheckLatestAppVersion();
	void UpdateAttributes(wxSpinEvent&);
	void UpdateSorting(wxCommandEvent&);
};