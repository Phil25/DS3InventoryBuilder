#include "Preview.h"

#include <AppMain.h>
#include <Context/IAttributesListener.h>
#include <Context/ISelectionListener.h>

class Preview::AttributesListener final : public IAttributesListener
{
	Preview* const preview;

public:
	AttributesListener(Preview* const preview) : preview(preview)
	{
	}

	void OnUpdate(const int str, const int dex, const int int_, const int fth, const int lck) override
	{
	}
};

class Preview::SelectionListener final : public ISelectionListener
{
	Preview* const preview;

public:
	SelectionListener(Preview* const preview) : preview(preview)
	{
	}

	void OnUpdate() override
	{
		preview->OnSelectionUpdate();
	}
};

Preview::Preview(wxWindow* parent)
	: Title(parent, "Weapon Preview")
	, attributesListener(std::make_shared<AttributesListener>(this))
	, selectionListener(std::make_shared<SelectionListener>(this))
	, label(new wxStaticText{GetContent(), wxID_ANY, "No weapons selected"})
{
	attributesListener->Register();
	selectionListener->Register();
}

void Preview::OnSelectionUpdate()
{
	const auto& selection = wxGetApp().GetSessionData().GetSelection();

	if (!selection.size())
	{
		label->SetLabel("No weapon selected");
	}
	else if (selection.size() == 1)
	{
		label->SetLabel("1 weapon selected");
	}
	else
	{
		label->SetLabel("Multiple weapons selected");
	}
}
