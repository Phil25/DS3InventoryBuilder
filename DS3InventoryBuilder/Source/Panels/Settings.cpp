#include "Settings.h"

#include <AppMain.h>
#include <Utils/DrawWeapon.h>
#include <Utils/InventoryEncoder.h>
#include <wx/spinctrl.h>
#include <wx/filedlg.h>
#include <wx/clipbrd.h>

namespace
{
	constexpr auto weaponIconSize = 128;

	auto CreateInventoryBitmap()
	{
		auto weakInventory = wxGetApp().GetSessionData().GetInventory();
		if (weakInventory.empty())
			return wxBitmap{};

		WeaponContext::Vector inventory;
		for (const auto& weakPtr : weakInventory)
			if (const auto& ptr = weakPtr.lock(); ptr)
				inventory.push_back(ptr);

		const int rows = inventory.size() / 5 + (inventory.size() % 5 != 0);
		auto bitmap = wxBitmap{5 * weaponIconSize, rows * weaponIconSize};
		auto dc = wxMemoryDC{bitmap};

		for (int pos = 0; const auto& context : inventory)
		{
			const int row = pos / 5;
			const int col = pos % 5;

			dc.SetBrush(wxColor{114,98,85});
			DrawWeapon(&dc, weaponIconSize, context->GetName(), context->GetInfusion(), {col * weaponIconSize, row * weaponIconSize}, static_cast<int>(context->GetRequirementsStatus()));

			++pos;
		}

		return bitmap;
	}

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
		//sizer->Add(new wxStaticText(this, wxID_ANY, "Inventory Sorting Method:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
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

class Settings::IOOperations final : public wxPanel
{
	class CodeDialog final : public wxDialog
	{
		wxStaticText* label;
		wxTextCtrl* text;
		wxButton* action;

	public:
		CodeDialog(wxWindow* parent, wxString dialogName, wxString labelName, wxString actionName, long extraStyle=0)
			: wxDialog(parent, wxID_ANY, std::move(dialogName), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
			, label(new wxStaticText{this, wxID_ANY, std::move(labelName)})
			, text(new wxTextCtrl{this, wxID_ANY, wxEmptyString, wxDefaultPosition, {260, 120}, wxTE_MULTILINE | extraStyle})
			, action(new wxButton{this, wxID_ANY, std::move(actionName)})
		{
			text->SetFocus();
			action->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { this->EndModal(wxID_APPLY); });

			auto* sizer = new wxBoxSizer(wxVERTICAL);
			sizer->Add(label, 0, wxEXPAND | wxALL, 5);
			sizer->Add(text, 1, wxEXPAND | wxALL, 5);
			sizer->Add(action, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 5);
			this->SetSizerAndFit(sizer);
		}

		auto GetInventoryCode() const
		{
			return text->GetValue().ToStdString();
		}

		void SetInventoryCode(const std::string& code)
		{
			text->SetValue(code);
		}
	};

	wxButton* encode;
	wxButton* decode;
	wxButton* save;
	wxButton* copy;

public:
	IOOperations(wxWindow* parent)
		: wxPanel(parent)
		, encode(new wxButton{this, wxID_ANY, wxT("Export Code")})
		, decode(new wxButton{this, wxID_ANY, wxT("Import Code")})
		, save(new wxButton{this, wxID_ANY, wxT("Save as PNG")})
		, copy(new wxButton{this, wxID_ANY, wxT("Copy PNG to Clipboard")})
	{
		encode->Bind(wxEVT_BUTTON, &IOOperations::OnEncode, this);
		decode->Bind(wxEVT_BUTTON, &IOOperations::OnDecode, this);
		save->Bind(wxEVT_BUTTON, &IOOperations::OnSave, this);
		copy->Bind(wxEVT_BUTTON, &IOOperations::OnCopy, this);

		auto* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(encode, 3, wxTOP, 3);
		sizer->Add(decode, 3, wxLEFT | wxTOP, 3);
		sizer->AddStretchSpacer(1);
		sizer->Add(save, 3, wxRIGHT | wxTOP, 3);
		sizer->Add(copy, 3, wxTOP, 3);
		this->SetSizer(sizer);
	}

private:
	void OnEncode(wxCommandEvent&)
	{
		auto dialog = CodeDialog{this, wxT("Export Code"), wxT("Import this code to recreate your inventory."), wxT("Copy to Clipboard"), wxTE_READONLY};

		const auto weapons = wxGetApp().GetSessionData().GetInventory();
		const auto inventoryCode = inventory_encoder::Encode(weapons);
		dialog.SetInventoryCode(inventoryCode);

		if (dialog.ShowModal() != wxID_APPLY)
			return;

		if (!wxTheClipboard->Open())
		{
			wxMessageDialog{nullptr, wxT("Failure accessing the system clipboard."), wxT("Export Code"), wxOK | wxICON_ERROR}.ShowModal();
			return;
		}

		wxTheClipboard->SetData(new wxTextDataObject(inventoryCode));
		wxTheClipboard->Flush();
		wxTheClipboard->Close();
	}

	void OnDecode(wxCommandEvent&)
	{
		auto dialog = CodeDialog{this, wxT("Import Code"), wxT("Paste your inventory code here.\nThis will OVERRIDE the current inventory!"), wxT("Apply")};
		if (dialog.ShowModal() != wxID_APPLY)
			return;

		try
		{
			const auto weapons = inventory_encoder::Decode(dialog.GetInventoryCode());
			wxGetApp().GetSessionData().OverrideWeapons(weapons);
		}
		catch (const inventory_encoder::Exception& e)
		{
			using E = inventory_encoder::Exception;
			switch (e)
			{
			case E::Empty: 
				wxMessageDialog{nullptr, wxT("Inventory code is empty."), wxT("Import Code"), wxOK | wxICON_EXCLAMATION}.ShowModal();
				break;

			case E::Invalid:
				wxMessageDialog{nullptr, wxT("Inventory code is invalid."), wxT("Import Code"), wxOK | wxICON_EXCLAMATION}.ShowModal();
				break;

			case E::RevisionNotSupported: 
				wxMessageDialog{nullptr, wxT("Inventory code was generated from a newer version."), wxT("Import Code"), wxOK | wxICON_EXCLAMATION}.ShowModal();
				break;
			}
		}
	}

	void OnSave(wxCommandEvent&)
	{
		const auto bitmap = CreateInventoryBitmap();
		if (!bitmap.IsOk())
		{
			wxMessageDialog{nullptr, wxT("Cannot create PNG from an empty inventory."), wxT("Copy PNG to Clipboard"), wxOK | wxICON_EXCLAMATION}.ShowModal();
			return;
		}

		auto dialog = wxFileDialog{this, wxT("Save as PNG"), wxEmptyString, wxT("MyInventory.png"), wxT("PNG files (*.png)|*.png|All files|*"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT};
		if (dialog.ShowModal() == wxID_CANCEL)
			return;

		if (!bitmap.SaveFile(dialog.GetPath(), wxBITMAP_TYPE_PNG))
			wxMessageDialog{nullptr, wxT("Error writing to file."), wxT("Save as PNG"), wxOK | wxICON_ERROR}.ShowModal();
	}

	void OnCopy(wxCommandEvent&)
	{
		const auto bitmap = CreateInventoryBitmap();
		if (!bitmap.IsOk())
		{
			wxMessageDialog{nullptr, wxT("Cannot create PNG from an empty inventory."), wxT("Copy PNG to Clipboard"), wxOK | wxICON_EXCLAMATION}.ShowModal();
			return;
		}

		if (!wxTheClipboard->Open())
		{
			wxMessageDialog{nullptr, wxT("Failure accessing the system clipboard."), wxT("Copy PNG to Clipboard"), wxOK | wxICON_ERROR}.ShowModal();
			return;
		}

		wxTheClipboard->SetData(new wxBitmapDataObject(bitmap));
		wxTheClipboard->Close();

		wxMessageDialog{nullptr, wxT("Inventory copied!\nBe sure to paste it before closing this application."), wxT("Copy PNG to Clipboard"), wxOK}.ShowModal();
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
	, ioOperations(new IOOperations{GetContent()})
{
	this->SetMinSize(wxSize(550, 180));

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
	sizer->Add(attributes, 0, wxEXPAND | wxALL, 10);
	sizer->Add(inventorySorting, 0, wxEXPAND | wxALL, 10);
	sizer->Add(ioOperations, 0, wxEXPAND | wxALL, 10);

	GetContent()->SetSizer(sizer);
}

void Settings::UpdateAttributes(wxSpinEvent&)
{
	wxGetApp().GetSessionData().UpdateAttributes(
		str->GetValue(), dex->GetValue(), int_->GetValue(), fth->GetValue(), lck->GetValue());
}

void Settings::UpdateSorting(wxCommandEvent&)
{
	using M = invbuilder::Weapon::Sorting::Method;
	auto sorting = inventorySorting->GetInventorySorting();
	static bool showAttackPowerWarning = true;

	if (showAttackPowerWarning && wxGetApp().GetSessionData().GetSorting().method != M::AttackPower && sorting.method == M::AttackPower)
	{
		const auto result = wxMessageDialog{nullptr, wxT(
			"Sorting by Attack Power is not 100% valid. "
			"If you know the details on how this sorting method is implemented in the game, please let me know.\n"
			"Click OK to not show this message for this session again."),
			wxT("Sort by Attack Power"), wxOK | wxCANCEL | wxICON_WARNING}.ShowModal();
		showAttackPowerWarning = result == wxID_CANCEL;
	}

	inventorySorting->SetTwoHandedVisible(sorting.method == M::AttackPower);
	wxGetApp().GetSessionData().UpdateInventorySorting(std::move(sorting));
}
