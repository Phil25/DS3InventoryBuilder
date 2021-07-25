#include "FrameMain.h"

#include <Panels/Finder.h>
#include <Panels/Inventory.h>
#include <Panels/Settings.h>
#include <Panels/Preview.h>

FrameMain::FrameMain(wxString title) : wxFrame(nullptr, wxID_ANY, std::move(title))
{
	auto* main = new wxBoxSizer(wxHORIZONTAL);
	auto* right = new wxBoxSizer(wxVERTICAL);

	auto* finder = new Finder(this);
	auto* inventory = new Inventory(this);
	auto* settings = new Settings(this);
	auto* preview = new Preview(this);

	right->Add(settings, 0, wxEXPAND | wxBOTTOM, 3);
	right->Add(preview, 1, wxEXPAND, 3);

	main->Add(finder, 1, wxEXPAND | wxALL, 3);
	main->Add(inventory, 1, wxEXPAND | wxRIGHT | wxTOP | wxBOTTOM, 3);
	main->Add(right, 0, wxEXPAND | wxRIGHT | wxTOP | wxBOTTOM, 3);

	this->SetSizerAndFit(main);
}

void FrameMain::NotifyOutdated()
{
	auto* bar = this->CreateStatusBar();
	bar->SetBackgroundColour(wxColor{255,78,80});
	bar->SetStatusText(wxT("Your DS3InventoryBuilder version is out of date!"));
}
