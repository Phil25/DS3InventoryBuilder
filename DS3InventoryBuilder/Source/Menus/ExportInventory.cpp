#include "ExportInventory.h"

#include <AppMain.h>
#include <Utils/DrawWeapon.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include <wx/clipbrd.h>

namespace
{
	constexpr auto weaponIconSize = 128;

	auto CreateInventoryBitmap()
	{
		auto weakInventory = wxGetApp().GetSessionData().GetInventory();
		if (weakInventory.empty())
			return wxBitmap{};

		std::vector<std::shared_ptr<WeaponContext>> inventory;
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
}

class EncoderPanel : public wxPanel
{
	wxTextCtrl* field;

public:
	EncoderPanel(wxWindow* parent)
		: wxPanel(parent)
		, field(new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY))
	{
		auto* sizer = new wxBoxSizer(wxVERTICAL);

		sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Encoded text form:")));
		sizer->Add(field, 1, wxEXPAND);
		sizer->Add(new wxButton(this, wxID_ANY, wxT("Copy to Clipboard")), 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, 5);

		this->SetSizer(sizer);
	}
};

ExportInventory::ExportInventory(wxWindow* parent)
	: wxDialog(parent, wxID_ANY, "Export Inventory")
{
	auto* save = new wxButton(this, wxID_ANY, wxT("Save as PNG"));
	save->Bind(wxEVT_BUTTON, &ExportInventory::SavePNG, this);

	auto* copy = new wxButton(this, wxID_ANY, wxT("Copy PNG to Clipboard"));
	copy->Bind(wxEVT_BUTTON, &ExportInventory::CopyPNG, this);

	auto* pngs = new wxBoxSizer(wxHORIZONTAL);
	pngs->Add(save, 1, wxRIGHT, 3);
	pngs->Add(copy, 1);

	auto* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(pngs, 0, wxALL, 10);
	sizer->Add(new EncoderPanel{this}, 0, wxEXPAND | wxALL, 10);

	this->SetSizerAndFit(sizer);
}

void ExportInventory::SavePNG(wxCommandEvent&)
{
}

void ExportInventory::CopyPNG(wxCommandEvent&)
{
	const auto bitmap = CreateInventoryBitmap();
	if (bitmap.IsOk() && wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(new wxBitmapDataObject(bitmap));
		wxTheClipboard->Close();
	}
}
