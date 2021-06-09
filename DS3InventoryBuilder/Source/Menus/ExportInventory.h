#pragma once

#include <wx/dialog.h>

class ExportInventory final : public wxDialog
{
public:
	ExportInventory(wxWindow* parent);

private:
	void SavePNG(wxCommandEvent&);
	void CopyPNG(wxCommandEvent&);
};