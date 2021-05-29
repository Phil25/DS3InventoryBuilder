#include "IAttributesListener.h"

#include <AppMain.h>

void IAttributesListener::Register()
{
	wxGetApp().GetSessionData().RegisterAttributesListener(shared_from_this());
}
