#pragma once

#include <wx/wx.h>

class FrameMain final : public wxFrame
{
public:
	FrameMain(wxString title);

	void NotifyOutdated(std::string latestVersion);
};