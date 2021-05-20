#include "AppMain.h"

wxIMPLEMENT_APP(AppMain);

AppMain::AppMain()
	: database(invbuilder::Database::Create())
{
}

bool AppMain::OnInit()
{
	wxInitAllImageHandlers();

	auto title = wxString("DS3InventoryBuilder -- v");
	title.Append(APP_VERSION);

	frameMain = new FrameMain(std::move(title));
	frameMain->Show();

	return true;
}

auto AppMain::GetDatabase() -> const invbuilder::Database&
{
	return database;
}
