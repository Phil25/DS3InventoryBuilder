#pragma once

#include <wx/dc.h>
#include <Weapon.hpp>

void DrawWeapon(wxDC* dc, const int size, const std::string& name, const invbuilder::Weapon::Infusion infusion, const wxPoint& pos, const int missingRequirements);