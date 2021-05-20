#include "Finder.h"

Finder::Finder(wxWindow* parent)
	: Title(parent, "Weapon Finder")
{
	this->SetMinSize(wxSize(64 * 5, 0)); // 5x 64x64 icons
	this->SetMaxSize(wxSize(128 * 5, 99999)); // 5x 128x128 icons
}
