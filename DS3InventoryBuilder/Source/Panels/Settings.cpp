#include "Settings.h"

#include <AppMain.h>
#include <wx/spinctrl.h>

class Settings::Attribute final : public wxPanel
{
	wxSpinCtrl* control;

public:
	Attribute(wxWindow* parent, const char* name, const int initial=10)
		: wxPanel(parent)
		, control(new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 99, initial))
	{
		auto* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(new wxStaticText(this, wxID_ANY, name), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
		sizer->Add(control);
		this->SetSizer(sizer);
	}

	auto GetValue()
	{
		return control->GetValue();
	}
};

Settings::Settings(wxWindow* parent)
	: Title(parent, "Settings")
	, str(new Attribute{GetContent(), "STR", 18})
	, dex(new Attribute{GetContent(), "DEX", 18})
	, int_(new Attribute{GetContent(), "INT", 10})
	, fth(new Attribute{GetContent(), "FTH", 10})
	, lck(new Attribute{GetContent(), "LCK", 7})
{
	this->SetMinSize(wxSize(550, 0));

	str->Bind(wxEVT_SPINCTRL, &Settings::UpdateAttributes, this);
	dex->Bind(wxEVT_SPINCTRL, &Settings::UpdateAttributes, this);
	int_->Bind(wxEVT_SPINCTRL, &Settings::UpdateAttributes, this);
	fth->Bind(wxEVT_SPINCTRL, &Settings::UpdateAttributes, this);
	lck->Bind(wxEVT_SPINCTRL, &Settings::UpdateAttributes, this);

	auto* attributes = new wxBoxSizer(wxHORIZONTAL);
	attributes->Add(str, 1);
	attributes->Add(dex, 1);
	attributes->Add(int_, 1);
	attributes->Add(fth, 1);
	attributes->Add(lck, 1);

	auto* sizer = new wxBoxSizer(wxVERTICAL);

	sizer->Add(attributes, 1, wxEXPAND, 0);

	GetContent()->SetSizer(sizer);
}

void Settings::UpdateAttributes(wxSpinEvent&)
{
	wxGetApp().GetSessionData().UpdateAttributes(
		str->GetValue(), dex->GetValue(), int_->GetValue(), fth->GetValue(), lck->GetValue());
}
