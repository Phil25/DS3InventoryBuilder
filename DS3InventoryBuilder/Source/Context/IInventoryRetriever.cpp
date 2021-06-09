#include "IInventoryRetriever.h"

#include <AppMain.h>

void IInventoryRetriever::Register()
{
	wxGetApp().GetSessionData().RegisterInventoryRetriever(shared_from_this());
}
