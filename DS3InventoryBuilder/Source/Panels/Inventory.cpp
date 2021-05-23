#include "Inventory.h"

Inventory::Inventory(wxWindow* parent)
	: Title(parent, "Inventory")
{
	this->SetMinSize(wxSize(64 * 5, 0)); // 5x 64x64 icons
}
