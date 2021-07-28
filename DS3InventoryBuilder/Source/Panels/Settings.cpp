#include "Settings.h"

#include <AppMain.h>
#include <Utils/DrawWeapon.h>
#include <Utils/InventoryEncoder.h>
#include <Menus/HelpLinks.h>
#include <wx/spinctrl.h>
#include <wx/filedlg.h>
#include <wx/clipbrd.h>
#include <wx/sstream.h>
#include <wx/webrequest.h>
#include <rapidjson/document.h>

namespace
{
	const auto exportCodeLabel = wxString{"Export Code"};
	const auto importCodeLabel = wxString{"Import Code"};
	const auto savePNGLabel = wxString{"Save PNG"};
	const auto copyPNGLabel = wxString{"Copy PNG"};
	const auto helpLabel = wxString{"Helpful Links"};

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

		int pos = 0;
		for (const auto& context : inventory)
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

namespace update_checker
{
	const auto url = wxString{"https://api.github.com/repos/Phil25/DS3InventoryBuilder/releases/latest"};

	std::string ParseVersion(const wxStringOutputStream& out)
	{
		rapidjson::Document doc;
		doc.Parse(out.GetString());

		const auto& tagName = doc["tag_name"];
		assert(tagName.IsString() && "tag_name from GitHub API should be a string");

		return tagName.GetString();
	}

	auto GetVersion(wxWebRequestEvent& e)
	{
		assert(e.GetState() == wxWebRequest::State_Completed && "wxWebRequest should have finished");

		wxStringOutputStream out;
		e.GetResponse().GetStream()->Read(out);

		return ParseVersion(out);
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

class Settings::MenuBar final : public wxPanel
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
	wxButton* help;
	bool hasLatestVersion{true};

public:
	MenuBar(wxWindow* parent)
		: wxPanel(parent)
		, encode(new wxButton{this, wxID_ANY, exportCodeLabel})
		, decode(new wxButton{this, wxID_ANY, importCodeLabel})
		, save(new wxButton{this, wxID_ANY, savePNGLabel})
		, copy(new wxButton{this, wxID_ANY, copyPNGLabel})
		, help(new wxButton{this, wxID_ANY, helpLabel})
	{
		encode->Bind(wxEVT_BUTTON, &MenuBar::OnEncode, this);
		decode->Bind(wxEVT_BUTTON, &MenuBar::OnDecode, this);
		save->Bind(wxEVT_BUTTON, &MenuBar::OnSave, this);
		copy->Bind(wxEVT_BUTTON, &MenuBar::OnCopy, this);
		help->Bind(wxEVT_BUTTON, &MenuBar::OnHelp, this);

		auto* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(encode, 3, wxTOP, 3);
		sizer->Add(decode, 3, wxLEFT | wxTOP, 3);

		sizer->AddStretchSpacer(1);
		sizer->Add(save, 3, wxRIGHT | wxTOP, 3);
		sizer->Add(copy, 3, wxTOP, 3);

		sizer->AddStretchSpacer(1);
		sizer->Add(help, 3, wxTOP, 3);

		this->SetSizer(sizer);
	}

	void SetHasLatestVersion(const bool val)
	{
		hasLatestVersion = val;

		if (!val)
		{
			help->Hide();
			help->SetBackgroundColour(wxColor{231,180,22});
			help->ShowWithEffect(wxSHOW_EFFECT_SLIDE_TO_LEFT);
		}
	}

private:
	void OnEncode(wxCommandEvent&)
	{
		auto dialog = CodeDialog{this, exportCodeLabel, wxT("Import this code to recreate your inventory."), wxT("Copy to Clipboard"), wxTE_READONLY};

		const auto weapons = wxGetApp().GetSessionData().GetInventory();
		const auto inventoryCode = inventory_encoder::Encode(weapons);
		dialog.SetInventoryCode(inventoryCode);

		if (dialog.ShowModal() != wxID_APPLY)
			return;

		if (!wxTheClipboard->Open())
		{
			wxMessageDialog{nullptr, wxT("Failure accessing the system clipboard."), exportCodeLabel, wxOK | wxICON_ERROR}.ShowModal();
			return;
		}

		wxTheClipboard->SetData(new wxTextDataObject(inventoryCode));
		wxTheClipboard->Flush();
		wxTheClipboard->Close();
	}

	void OnDecode(wxCommandEvent&)
	{
		auto dialog = CodeDialog{this, importCodeLabel, wxT("Paste your inventory code here.\nThis will OVERRIDE the current inventory!"), wxT("Apply")};
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
				wxMessageDialog{nullptr, wxT("Inventory code is empty."), importCodeLabel, wxOK | wxICON_EXCLAMATION}.ShowModal();
				break;

			case E::Invalid:
				wxMessageDialog{nullptr, wxT("Inventory code is invalid."), importCodeLabel, wxOK | wxICON_EXCLAMATION}.ShowModal();
				break;

			case E::RevisionNotSupported: 
				wxMessageDialog{nullptr, wxT("Inventory code is invalid or generated from a newer version."), importCodeLabel, wxOK | wxICON_EXCLAMATION}.ShowModal();
				break;
			}
		}
	}

	void OnSave(wxCommandEvent&)
	{
		const auto bitmap = CreateInventoryBitmap();
		if (!bitmap.IsOk())
		{
			wxMessageDialog{nullptr, wxT("Cannot create PNG from an empty inventory."), savePNGLabel, wxOK | wxICON_EXCLAMATION}.ShowModal();
			return;
		}

		auto dialog = wxFileDialog{this, savePNGLabel, wxEmptyString, wxT("MyInventory.png"), wxT("PNG files (*.png)|*.png|All files|*"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT};
		if (dialog.ShowModal() == wxID_CANCEL)
			return;

		if (!bitmap.SaveFile(dialog.GetPath(), wxBITMAP_TYPE_PNG))
			wxMessageDialog{nullptr, wxT("Error writing to file."), savePNGLabel, wxOK | wxICON_ERROR}.ShowModal();
	}

	void OnCopy(wxCommandEvent&)
	{
		const auto bitmap = CreateInventoryBitmap();
		if (!bitmap.IsOk())
		{
			wxMessageDialog{nullptr, wxT("Cannot create PNG from an empty inventory."), copyPNGLabel, wxOK | wxICON_EXCLAMATION}.ShowModal();
			return;
		}

		if (!wxTheClipboard->Open())
		{
			wxMessageDialog{nullptr, wxT("Failure accessing the system clipboard."), copyPNGLabel, wxOK | wxICON_ERROR}.ShowModal();
			return;
		}

		wxTheClipboard->SetData(new wxBitmapDataObject(bitmap));
		wxTheClipboard->Close();

		wxMessageDialog{nullptr, wxT("Inventory copied!\nBe sure to paste it before closing this application."), copyPNGLabel, wxOK}.ShowModal();
	}

	void OnHelp(wxCommandEvent&)
	{
		auto menu = HelpLinks{hasLatestVersion};
		const auto pos = help->GetPosition();
		const auto size = help->GetSize();

		PopupMenu(&menu, {pos.x, pos.y + size.GetHeight()});
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
	, menuBar(new MenuBar{GetContent()})
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
	sizer->Add(menuBar, 0, wxEXPAND | wxALL, 10);

	GetContent()->SetSizer(sizer);

	CheckLatestAppVersion();
}

void Settings::CheckLatestAppVersion()
{
	auto request = wxWebSession::GetDefault().CreateRequest(this, update_checker::url);
	if (!request.IsOk())
		return;

	this->Bind(wxEVT_WEBREQUEST_STATE, [&](wxWebRequestEvent& e)
	{
		if (e.GetState() == wxWebRequest::State_Completed)
			this->menuBar->SetHasLatestVersion(update_checker::GetVersion(e) == APP_VERSION);
	});

	request.Start();
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
