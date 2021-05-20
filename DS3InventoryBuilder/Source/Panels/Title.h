#pragma once

#include <wx/wx.h>

class Title : public wxPanel
{
	wxPanel* content{nullptr};

public:
	Title(wxWindow* parent, const char* titleText, int sizeText=14);

	auto GetContent() -> wxPanel*;
};
