#include "Finder.h"

#include <Context/ISelectionListener.h>
#include <Context/IAttributesListener.h>
#include <Panels/WeaponGrid.h>
#include <Database.h>
#include <wx/popupwin.h>
#include <wx/srchctrl.h>

class Finder::AttributesListener final : public IAttributesListener
{
	Finder* const finder;

public:
	AttributesListener(Finder* const finder) : finder(finder)
	{
	}

	void OnUpdate(const int str, const int dex, const int int_, const int fth, const int lck) override
	{
		finder->grid->UpdateRequirements();
	}
};

class Finder::SelectionListener final : public ISelectionListener
{
	Finder* const finder;

public:
	SelectionListener(Finder* const finder) : finder(finder)
	{
	}

	void OnUpdate(const int gridID) override
	{
		if (gridID != finder->grid->gridID)
			finder->grid->DiscardSelection();
	}
};

class Finder::FilterControls final : public wxPanel
{
	using Type = invbuilder::Weapon::Type;
	using Infusion = invbuilder::Weapon::Infusion;
	using Sorting = invbuilder::Weapon::Sorting;
	using DB = invbuilder::Database;

	template <class Property>
	class Group final : public wxPanel
	{
		wxCheckBox* all;
		const int start;

		std::vector<wxCheckBox*> boxes;

	public:
		Group(wxWindow* parent, wxString name, const Property start, const Property end, const bool startSelected=false)
			: wxPanel(parent)
			, all(new wxCheckBox(this, wxID_ANY, std::move(name), wxDefaultPosition, wxDefaultSize, wxCHK_3STATE))
			, start(static_cast<int>(start))
		{
			all->SetFont(all->GetFont().MakeLarger().MakeLarger().MakeUnderlined());
			all->SetValue(startSelected);

			auto* sizer = new wxBoxSizer(wxVERTICAL);

			sizer->Add(all);
			for (int i = this->start; i <= static_cast<int>(end); ++i)
			{
				auto* box = new wxCheckBox(this, wxID_ANY, DB::ToString(static_cast<Property>(i)));
				box->SetValue(startSelected);

				boxes.emplace_back(box);
				sizer->Add(box);
			}

			this->SetSizer(sizer);

			this->Bind(wxEVT_CHECKBOX, &Group::OnCheckBox, this);
		}

		void GetSelection(std::set<Property>& out) const
		{
			int offset = 0;
			for (const auto* box : boxes)
			{
				if (box->IsChecked())
					out.emplace(static_cast<Property>(start + offset));

				++offset;
			}
		}

	private:
		void OnCheckBox(wxCommandEvent& e)
		{
			if (auto* obj = e.GetEventObject(); obj == all)
				SetAllSelected(all->GetValue());
			else
				all->Set3StateValue(GetCheckBoxState());

			e.Skip(true);
		}

		void SetAllSelected(const bool check)
		{
			for (auto* box : boxes)
				box->SetValue(check);
		}

		auto GetCheckBoxState() const -> wxCheckBoxState
		{
			const auto size = boxes.size();
			if (!size)
				return wxCHK_UNCHECKED;

			const bool first = boxes[0]->GetValue();

			for (size_t i = 1; i < size; ++i)
				if (boxes[i]->GetValue() != first)
					return wxCHK_UNDETERMINED;

			return first ? wxCHK_CHECKED : wxCHK_UNCHECKED;
		}
	};

	class TypeFilter final : public wxPopupTransientWindow
	{
		Group<Type>* melee;
		Group<Type>* ranged;
		Group<Type>* castingTools;
		Group<Type>* shields;

	public:
		TypeFilter(wxWindow* parent)
			: wxPopupTransientWindow(parent, wxBORDER_THEME)
			, melee(new Group(this, "Melee", Type::Dagger, Type::Claw, true))
			, ranged(new Group(this, "Ranged", Type::Bow, Type::Crossbow, true))
			, castingTools(new Group(this, "Casting Tools", Type::Staff, Type::SacredChime, true))
			, shields(new Group(this, "Shields", Type::Torch, Type::Greatshield, true))
		{
			auto* sizer = new wxBoxSizer(wxHORIZONTAL);

			sizer->Add(melee, 0, wxEXPAND | wxALL, 3);
			sizer->Add(ranged, 0, wxEXPAND | wxALL, 3);
			sizer->Add(castingTools, 0, wxEXPAND | wxALL, 3);
			sizer->Add(shields, 0, wxEXPAND | wxALL, 3);

			this->SetSizerAndFit(sizer);
		}

		auto GetWeaponTypes() const
		{
			std::set<Type> types;
			melee->GetSelection(types);
			ranged->GetSelection(types);
			castingTools->GetSelection(types);
			shields->GetSelection(types);
			return types;
		}
	};

	class InfusionFilter final : public wxPopupTransientWindow
	{
		Group<Infusion>* melee;
		Group<Infusion>* ranged;
		Group<Infusion>* castingTools;
		Group<Infusion>* shields;

	public:
		InfusionFilter(wxWindow* parent)
			: wxPopupTransientWindow(parent, wxBORDER_THEME)
			, melee(new Group(this, "None", Infusion::None, Infusion::None, true))
			, ranged(new Group(this, "Physical", Infusion::Heavy, Infusion::Raw))
			, castingTools(new Group(this, "Elemental", Infusion::Crystal, Infusion::Dark))
			, shields(new Group(this, "Luck", Infusion::Blood, Infusion::Hollow))
		{
			auto* sizer = new wxBoxSizer(wxHORIZONTAL);

			sizer->Add(melee, 0, wxEXPAND | wxALL, 3);
			sizer->Add(ranged, 0, wxEXPAND | wxALL, 3);
			sizer->Add(castingTools, 0, wxEXPAND | wxALL, 3);
			sizer->Add(shields, 0, wxEXPAND | wxALL, 3);

			this->SetSizerAndFit(sizer);
		}

		auto GetWeaponInfusions() const
		{
			std::set<Infusion> infusions;
			melee->GetSelection(infusions);
			ranged->GetSelection(infusions);
			castingTools->GetSelection(infusions);
			shields->GetSelection(infusions);
			return infusions;
		}
	};

	Finder* const finder;

	wxSearchCtrl* filter;
	wxButton* openTypeFilter;
	wxButton* openInfusionFilter;
	wxButton* order;
	wxButton* level;

	TypeFilter* typeFilter;
	InfusionFilter* infusionFilter;

	std::set<Type> types{};
	std::set<Infusion> infusions{Infusion::None};
	Sorting sorting{};

public:
	FilterControls(wxWindow* parent, Finder* finder)
		: wxPanel(parent)
		, finder(finder)
		, filter(new wxSearchCtrl(this, wxID_ANY))
		, order(new wxButton(this, wxID_ANY, wxT("Order")))
		, openTypeFilter(new wxButton(this, wxID_ANY, wxT("Classes")))
		, openInfusionFilter(new wxButton(this, wxID_ANY, wxT("Infusions")))
		, level(new wxButton(this, wxID_ANY, wxT("+10")))
		, typeFilter(new TypeFilter(this))
		, infusionFilter(new InfusionFilter(this))
	{
		for (int type = 0; type < static_cast<int>(Type::Size); ++type)
			types.emplace(static_cast<Type>(type));

		this->SetMaxSize(wxSize{128 * 5, 99999});

		openTypeFilter->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { this->ShowPopup(typeFilter, openTypeFilter); });
		typeFilter->Bind(wxEVT_CHECKBOX, &FilterControls::OnType, this);

		openInfusionFilter->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { this->ShowPopup(infusionFilter, openInfusionFilter); });
		infusionFilter->Bind(wxEVT_CHECKBOX, &FilterControls::OnInfusion, this);

		filter->SetFocus();
		filter->SetDescriptiveText(wxT("Filter..."));

		auto* sizer = new wxBoxSizer(wxHORIZONTAL);

		sizer->Add(filter, 6, wxALIGN_CENTRE_VERTICAL);
		sizer->AddStretchSpacer(1);
		sizer->Add(openTypeFilter, 4, wxALIGN_CENTRE_VERTICAL);
		sizer->Add(openInfusionFilter, 4, wxALIGN_CENTRE_VERTICAL);
		sizer->AddStretchSpacer(1);
		sizer->Add(order, 4, wxALIGN_CENTRE_VERTICAL);
		sizer->Add(level, 1, wxALIGN_CENTRE_VERTICAL);

		this->SetSizer(sizer);
	}

	const auto& GetWeaponTypes() const
	{
		return types;
	}

	const auto& GetWeaponInfusions() const
	{
		return infusions;
	}

	const auto& GetWeaponSorting() const
	{
		return sorting;
	}

private:
	void ShowPopup(wxPopupTransientWindow* window, const wxButton* button)
	{
		auto pos = button->GetScreenPosition() + wxSize{0, button->GetSize().y};
		window->SetPosition(pos);
		window->Show();
	}

	void OnType(wxCommandEvent&)
	{
		types = typeFilter->GetWeaponTypes();
		finder->OnFilterControlsUpdate();
	}

	void OnInfusion(wxCommandEvent&)
	{
		infusions = infusionFilter->GetWeaponInfusions();
		finder->OnFilterControlsUpdate();
	}
};

Finder::Finder(wxWindow* parent)
	: Title(parent, "Weapon Finder")
	, attributesListener(std::make_shared<AttributesListener>(this))
	, selectionListener(std::make_shared<SelectionListener>(this))
	, controls(new FilterControls(GetContent(), this))
	, grid(new WeaponGrid(GetContent()))
{
	attributesListener->Register();
	selectionListener->Register();

	grid->InitializeAllWeapons();

	auto* sizerContent = new wxBoxSizer(wxVERTICAL);
	sizerContent->Add(controls, 0, wxEXPAND | wxBOTTOM, 10);
	sizerContent->Add(grid, 1, wxEXPAND);

	auto* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->AddStretchSpacer(1);
	sizer->Add(sizerContent, 1, wxEXPAND);
	sizer->AddStretchSpacer(1);

	GetContent()->SetSizer(sizer);

	OnFilterControlsUpdate();
}

void Finder::OnFilterControlsUpdate()
{
	grid->SetFiltering(
		controls->GetWeaponTypes(),
		controls->GetWeaponInfusions(),
		controls->GetWeaponSorting()
	);
}
