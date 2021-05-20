#pragma once

#include <Database.h>
#include <Frames/FrameMain.h>
#include <wx/wx.h>

class AppMain final : public wxApp
{
	const invbuilder::Database database;
	FrameMain* frameMain{nullptr};

public:
	AppMain();

	bool OnInit() override;

	auto GetDatabase() -> const invbuilder::Database&;
};

DECLARE_APP(AppMain)
