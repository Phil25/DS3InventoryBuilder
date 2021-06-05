#include "Settings.h"

#include <AppMain.h>
#include <wx/spinctrl.h>

namespace
{
	auto GetSortingMethodChoices()
	{
		using M = invbuilder::Weapon::Sorting::Method;
		wxArrayString arr;

		for (int i = 0; i < static_cast<int>(M::Size); ++i)
			arr.Add(invbuilder::Database::ToString(static_cast<M>(i)));

		return arr;
	}
}

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

	auto GetValue() const
	{
		return control->GetValue();
	}
};

class Settings::InventorySorting final : public wxPanel
{
	wxChoice* method;
	wxCheckBox* reversed;
	wxCheckBox* twoHanded;

public:
	InventorySorting(wxWindow* parent)
		: wxPanel(parent)
		, method(new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, GetSortingMethodChoices()))
		, reversed(new wxCheckBox(this, wxID_ANY, wxT("Reversed")))
		, twoHanded(new wxCheckBox(this, wxID_ANY, wxT("Two Handed")))
	{
		method->SetSelection(0);
		twoHanded->Disable();

		auto* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(new wxStaticText(this, wxID_ANY, "Inventory Sorting Method:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
		sizer->Add(method, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
		sizer->Add(reversed, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
		sizer->Add(twoHanded, 0, wxALIGN_CENTER_VERTICAL);
		this->SetSizer(sizer);
	}

	auto GetInventorySorting() const -> invbuilder::Weapon::Sorting
	{
		return {
			static_cast<invbuilder::Weapon::Sorting::Method>(method->GetSelection()),
			reversed->GetValue(),
			twoHanded->GetValue()
		};
	}

	void SetTwoHandedVisible(const bool set)
	{
		if (set) twoHanded->Enable();
		else twoHanded->Disable();
	}
};

Settings::Settings(wxWindow* parent)
	: Title(parent, "Settings")
	, str(new Attribute{GetContent(), "STR", 18})
	, dex(new Attribute{GetContent(), "DEX", 18})
	, int_(new Attribute{GetContent(), "INT", 10})
	, fth(new Attribute{GetContent(), "FTH", 10})
	, lck(new Attribute{GetContent(), "LCK", 7})
	, inventorySorting(new InventorySorting{GetContent()})
{
	this->SetMinSize(wxSize(550, 140));

	str->Bind(wxEVT_SPINCTRL, &Settings::UpdateAttributes, this);
	dex->Bind(wxEVT_SPINCTRL, &Settings::UpdateAttributes, this);
	int_->Bind(wxEVT_SPINCTRL, &Settings::UpdateAttributes, this);
	fth->Bind(wxEVT_SPINCTRL, &Settings::UpdateAttributes, this);
	lck->Bind(wxEVT_SPINCTRL, &Settings::UpdateAttributes, this);

	inventorySorting->Bind(wxEVT_CHOICE, &Settings::UpdateSorting, this);
	inventorySorting->Bind(wxEVT_CHECKBOX, &Settings::UpdateSorting, this);

	auto* attributes = new wxBoxSizer(wxHORIZONTAL);
	attributes->Add(str, 1);
	attributes->Add(dex, 1);
	attributes->Add(int_, 1);
	attributes->Add(fth, 1);
	attributes->Add(lck, 1);

	auto* sizer = new wxBoxSizer(wxVERTICAL);

	sizer->Add(attributes, 0, wxEXPAND | wxALL, 5);
	sizer->Add(inventorySorting, 0, wxEXPAND | wxALL, 5);

	GetContent()->SetSizer(sizer);
}

void Settings::UpdateAttributes(wxSpinEvent&)
{
	wxGetApp().GetSessionData().UpdateAttributes(
		str->GetValue(), dex->GetValue(), int_->GetValue(), fth->GetValue(), lck->GetValue());
}

void Settings::UpdateSorting(wxCommandEvent&)
{
	auto sorting = inventorySorting->GetInventorySorting();
	inventorySorting->SetTwoHandedVisible(sorting.method == invbuilder::Weapon::Sorting::Method::AttackPower);
	wxGetApp().GetSessionData().UpdateInventorySorting(std::move(sorting));
}
