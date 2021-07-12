#include "WeaponPopup.h"

#include <AppMain.h>
#include <Database.h>
#include <fmt/core.h>

WeaponPopup::WeaponPopup(const GridRole role, const int selectedLevels, const int selectedInfusions)
	: role(role)
{
	if (role == GridRole::Browser)
	{
		this->Append(TransferSingle, wxT("Add"));
		this->Append(TransferRow, wxT("Add row"));
		this->Append(Transfer2Rows, wxT("Add 2 rows"));
		this->Append(TransferPage, wxT("Add page"));
	}
	else
	{
		this->Append(TransferSingle, wxT("Remove"));

		this->AppendSeparator();

		this->Append(DuplicateSingle, wxT("Duplicate"));
		this->Append(DuplicateRow, wxT("Make row"));
		this->Append(Duplicate2Rows, wxT("Make 2 rows"));
		this->Append(DuplicatePage, wxT("Make page"));

		this->AppendSeparator();

		this->AppendSubMenu(CreateLevelSubmenu(selectedLevels), wxT("Level"));
		this->AppendSubMenu(CreateInfusionSubmenu(selectedInfusions), wxT("Infusion"));
	}

	this->AppendSeparator();
	this->Append(SelectAll, wxT("Select All"));

	this->Bind(wxEVT_COMMAND_MENU_SELECTED, &WeaponPopup::OnSelection, this);
}

bool WeaponPopup::ShouldSelectAll() const
{
	return selection == SelectAll;
}

bool WeaponPopup::WereWeaponsTransferred() const
{
	return selection >= TransferSingle && selection <= TransferPage;
}

void WeaponPopup::OnSelection(wxCommandEvent& e)
{
	selection = static_cast<Selection>(e.GetId());

	if (selection < LevelOffset)
	{
		if (selection >= TransferSingle && selection <= TransferPage)
			wxGetApp().GetSessionData().UpdateWeaponTransfer(OtherGrid(role), GetTransferCount(selection));
		else if (selection >= DuplicateSingle && selection <= DuplicatePage)
			wxGetApp().GetSessionData().UpdateWeaponTransfer(role, GetTransferCount(selection));

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

inline auto WeaponPopup::CreateLevelSubmenu(const int selected) -> wxMenu*
{
	auto* levels = new wxMenu{};

	for (int i = 0; i <= 10; ++i)
	{
		auto* item = levels->AppendCheckItem(LevelOffset + i, fmt::format("+{}", i));
		if (1 << i  & selected) item->Check();
	}

	return levels;
}

inline auto WeaponPopup::CreateInfusionSubmenu(const int selected) -> wxMenu*
{
	using Infusion = invbuilder::Weapon::Infusion;
	auto* infusions = new wxMenu{};

	for (int i = 0; i < static_cast<int>(Infusion::Size); ++i)
	{
		auto* item = infusions->AppendCheckItem(InfusionOffset + i, invbuilder::Database::ToString(static_cast<Infusion>(i)));
		if (1 << i & selected) item->Check();
	}

	return infusions;
}

inline auto WeaponPopup::GetTransferCount(const Selection selection) -> int
{
	switch(selection)
	{
	case TransferSingle: case DuplicateSingle: return 1;
	case DuplicateRow: return 4;
	case TransferRow: return 5;
	case Duplicate2Rows: return 2 * 5 - 1;
	case Transfer2Rows: return 2 * 5;
	case DuplicatePage: return 5 * 5 - 1;
	case TransferPage: return 5 * 5;
	}

	assert(false && "invalid selection for transfer count");
	return 1;
}
