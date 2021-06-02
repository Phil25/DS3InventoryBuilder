#include "WeaponPopup.h"

#include <AppMain.h>
#include <Database.h>
#include <format>

WeaponPopup::WeaponPopup(const int gridID, const bool fixed, const int selectedLevels, const int selectedInfusions)
	: gridID(gridID)
{
	using Infusion = invbuilder::Weapon::Infusion;

	auto* levels = new wxMenu{};
	for (int i = 0; i <= 10; ++i)
	{
		auto* item = levels->AppendCheckItem(LevelOffset + i, std::format("+{}", i));
		if (1 << i  & selectedLevels) item->Check();
	}

	auto* infusions = new wxMenu{};
	for (int i = 0; i < static_cast<int>(Infusion::Size); ++i)
	{
		auto* item = infusions->AppendCheckItem(InfusionOffset + i, invbuilder::Database::ToString(static_cast<Infusion>(i)));
		if (1 << i & selectedInfusions) item->Check();
	}

	if (fixed)
	{
		this->Append(TransferSingle, wxT("Add"));
		this->Append(TransferRow, wxT("Add row"));
		this->Append(Transfer2Rows, wxT("Add 2 rows"));
		this->Append(TransferPage, wxT("Add page"));
	}
	else
	{
		this->Append(TransferSingle, wxT("Remove"));
	}

	this->AppendSeparator();

	this->AppendSubMenu(levels, wxT("Level"));
	this->AppendSubMenu(infusions, wxT("Infusion"));

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, &WeaponPopup::OnSelection, this);
}

void WeaponPopup::OnSelection(wxCommandEvent& e)
{
	const auto selection = static_cast<Selection>(e.GetId());

	if (selection < LevelOffset)
	{
		wxGetApp().GetSessionData().UpdateWeaponTransfer(gridID, GetTransferCount(selection));
		return;
	}

	const auto& weapons = wxGetApp().GetSessionData().GetSelection();

	if (selection < InfusionOffset)
	{
		const auto level = selection - LevelOffset;

		for (const auto& weakPtr : weapons)
			if (const auto& ptr = weakPtr.lock(); ptr)
				ptr->SetLevel(level);
	}
	else
	{
		const auto infusion = static_cast<invbuilder::Weapon::Infusion>(selection - InfusionOffset);

		for (const auto& weakPtr : weapons)
			if (const auto& ptr = weakPtr.lock(); ptr)
				ptr->SetInfusion(infusion);
	}

	wxGetApp().GetSessionData().UpdateSelection();
}

auto WeaponPopup::GetTransferCount(const Selection selection) -> int
{
	switch(selection)
	{
	case TransferSingle: return 1;
	case TransferRow: return 5;
	case Transfer2Rows: return 2 * 5;
	case TransferPage: return 5 * 5;
	}

	assert(false && "invalid selection for transfer count");
	return 1;
}
