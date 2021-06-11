#include "DrawWeapon.h"

#include <AppMain.h>

using Infusion = invbuilder::Weapon::Infusion;
using DB = invbuilder::Database;

void DrawWeapon(wxDC* dc, const int size, const std::string& name, const Infusion infusion, const wxPoint& pos, const int missingRequirements)
{
	dc->DrawRectangle(pos, {size, size});
	dc->DrawBitmap(wxGetApp().GetImage(name, size), pos, false);

	const auto iconSize = std::min(size / 4, 30);

	if (infusion != Infusion::None)
	{
		const int offset = size - iconSize - 3;
		dc->DrawBitmap(wxGetApp().GetImage(DB::ToString(infusion), iconSize), pos.x + offset, pos.y + offset, false);
	}

	switch (missingRequirements)
	{
	case 0: dc->DrawBitmap(wxGetApp().GetImage("NoStats", iconSize), pos.x + size - iconSize - 2, pos.y + 2, false); break;
	case 1: dc->DrawBitmap(wxGetApp().GetImage("TwoHanded", iconSize), pos.x + size - iconSize - 2, pos.y + 2, false); break;
	}
}
