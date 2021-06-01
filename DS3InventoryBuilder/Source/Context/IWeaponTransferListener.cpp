#include "IWeaponTransferListener.h"

#include <AppMain.h>

void IWeaponTransferListener::Register()
{
	wxGetApp().GetSessionData().RegisterWeaponTransferListener(shared_from_this());;
}
