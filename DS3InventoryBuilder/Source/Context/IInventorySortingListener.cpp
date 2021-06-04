#include "IInventorySortingListener.h"

#include <AppMain.h>

void IInventorySortingListener::Register()
{
	wxGetApp().GetSessionData().RegisterInventorySortingListener(shared_from_this());;
}
