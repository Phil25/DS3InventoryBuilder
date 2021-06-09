#include "DrawWeapon.h"

#include <AppMain.h>

using Infusion = invbuilder::Weapon::Infusion;
using DB = invbuilder::Database;

void DrawWeapon(wxDC* dc, const int size, const std::string& name, const Infusion infusion, const wxPoint& pos, const int missingRequirements)
{
	dc->DrawRectangle(pos, {size, size});

	dc->DrawBitmap(wxGetApp().GetImage(name, size), pos, false);

	if (infusion != Infusion::None)
	{
		const int infusionSize = size / 4;
		const int offset = size - infusionSize - 3;
		dc->DrawBitmap(wxGetApp().GetImage(DB::ToString(infusion), infusionSize), pos.x + offset, pos.y + offset, false);
	}

	switch (missingRequirements)
	{
	case 0: dc->DrawBitmap(wxGetApp().GetImage("NoStats", size / 4), pos.x + size - size / 4 - 2, pos.y + 2, false); break;
	case 1: dc->DrawBitmap(wxGetApp().GetImage("TwoHanded", size / 4), pos.x + size - size / 4 - 2, pos.y + 2, false); break;
	}
}
