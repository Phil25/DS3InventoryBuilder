#include "Title.h"

Title::Title(wxWindow* parent, const char* titleText, int sizeText)
	: wxPanel(parent, wxID_ANY)
	, content(new wxPanel(this, wxID_ANY))
{
	auto* sizer = new wxBoxSizer(wxVERTICAL);
	auto* title = new wxStaticText(this, wxID_ANY, titleText, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);

	title->SetFont(wxFont(sizeText, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_MEDIUM));
	title->SetBackgroundColour(wxColor(200, 200, 200));

	content->SetMinSize(wxSize{200, 200});

	sizer->Add(title, 0, wxEXPAND | wxALL, 5);
	sizer->Add(content, 1, wxEXPAND | wxALL, 5);

	this->SetSizerAndFit(sizer);
}

auto Title::GetContent() -> wxPanel*
{
	return content;
}
