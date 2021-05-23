#include "Settings.h"

Settings::Settings(wxWindow* parent)
	: Title(parent, "Global Settings")
{
	this->SetMinSize(wxSize(600, 0));
}
