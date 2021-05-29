#include "ISelectionListener.h"

#include <AppMain.h>

void ISelectionListener::Register()
{
	wxGetApp().GetSessionData().RegisterSelectionListener(shared_from_this());
}
