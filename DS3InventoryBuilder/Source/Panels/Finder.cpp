#include "Finder.h"

#include <Context/ISelectionListener.h>
#include <Context/IAttributesListener.h>
#include <Panels/WeaponGrid.h>
#include <Database.h>
#include <wx/popupwin.h>
#include <wx/srchctrl.h>
#include <fmt/core.h>

class Finder::AttributesListener final : public IAttributesListener
{
	Finder* const finder;

public:
	AttributesListener(Finder* const finder) : finder(finder)
	{
	}

	void OnUpdate(const int str, const int dex, const int int_, const int fth, const int lck) override
	{
		using M = invbuilder::Weapon::Sorting::Method;
		const auto& method = finder->sorting.method;

		if (method == M::AttackPower || method == M::AttackPowerPrecise ||
			method == M::AttackPowerPreciseTwoHanded || method == M::AttackPowerPreciseTwoHandedIfRequired)
			finder->grid->Sort(finder->sorting);

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

	class SortingFilter final : public wxPopupTransientWindow
	{
		auto GetMethods()
		{
			using M = Sorting::Method;
			wxArrayString choices;

			for (int m = 0; m < static_cast<int>(M::Size); ++m)
				choices.Add(DB::ToString(static_cast<M>(m)));

			return choices;
		}

		auto GetARCalculations()
		{
			wxArrayString choices;
			choices.Add(wxT("In-Game, as Inventory"));
			choices.Add(wxT("Precise, One Handed"));
			choices.Add(wxT("Precise, Two Handed"));
			choices.Add(wxT("Precise, Two Handed If Required"));
			return choices;
		}

		wxRadioBox* methods;

		wxCheckBox* reverse;
		wxCheckBox* stability;
		wxRadioBox* arCalculation;

		Sorting::Method attackPowerMethod{Sorting::Method::AttackPower};
		Sorting::Method guardAbsorptionMethod{Sorting::Method::GuardAbsorption};
		Sorting sorting{};

	public:
		SortingFilter(wxWindow* parent)
			: wxPopupTransientWindow(parent, wxBORDER_THEME)
			, methods(new wxRadioBox(this, wxID_ANY, wxT("Methods"), wxDefaultPosition, wxDefaultSize, GetMethods(), 1))
			, reverse(new wxCheckBox(this, wxID_ANY, wxT("Reversed")))
			, stability(new wxCheckBox(this, wxID_ANY, wxT("Stability First")))
			, arCalculation(new wxRadioBox(this, wxID_ANY, wxT("AR Calculation"), wxDefaultPosition, wxDefaultSize, GetARCalculations(), 1))
		{
			stability->Disable();
			arCalculation->Disable();

			reverse->Bind(wxEVT_CHECKBOX, &SortingFilter::OnReverse, this);
			methods->Bind(wxEVT_RADIOBOX, &SortingFilter::OnMethod, this);
			stability->Bind(wxEVT_CHECKBOX, &SortingFilter::OnStability, this);
			arCalculation->Bind(wxEVT_RADIOBOX, &SortingFilter::OnARCalculation, this);

			auto* options = new wxBoxSizer(wxVERTICAL);
			options->Add(reverse, 0, wxLEFT, 5);
			options->Add(stability, 0, wxLEFT, 5);
			options->Add(arCalculation, 0, wxTOP | wxTOP, 5);

			auto* sizer = new wxBoxSizer(wxHORIZONTAL);
			sizer->Add(methods, 1, wxEXPAND | wxALL, 5);
			sizer->Add(options, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

			this->SetSizerAndFit(sizer);
		}

		const auto& GetWeaponSorting()
		{
			return sorting;
		}

	private:
		void OnReverse(wxCommandEvent& e)
		{
			sorting.reverse = reverse->IsChecked();
			e.Skip(true);
		}

		void OnMethod(wxCommandEvent& e)
		{
			using M = Sorting::Method;

			const int selection = methods->GetSelection();
			assert(0 <= selection && selection < static_cast<int>(M::Size) && "invalid finder sorting method selection");

			const auto method = static_cast<M>(selection);
			switch (method)
			{
			case M::AttackPower:
				arCalculation->Enable();
				stability->Disable();
				sorting.method = attackPowerMethod;
				break;

			case M::GuardAbsorption:
				stability->Enable();
				arCalculation->Disable();
				sorting.method = guardAbsorptionMethod;
				break;

			default:
				stability->Disable();
				arCalculation->Disable();
				sorting.method = method;
				break;
			}

			e.Skip(true);
		}

		void OnStability(wxCommandEvent& e)
		{
			using M = Sorting::Method;

			guardAbsorptionMethod = stability->IsChecked() ? M::StabilityThenGuardAbsorption : M::GuardAbsorption;
			sorting.method = guardAbsorptionMethod;

			e.Skip(true);
		}

		void OnARCalculation(wxCommandEvent& e)
		{
			using M = Sorting::Method;

			switch (arCalculation->GetSelection())
			{
			case 0:
				attackPowerMethod = M::AttackPower;
				break;

			case 1:
				attackPowerMethod = M::AttackPowerPrecise;
				break;

			case 2:
				attackPowerMethod = M::AttackPowerPreciseTwoHanded;
				break;

			case 3:
				attackPowerMethod = M::AttackPowerPreciseTwoHandedIfRequired;
				break;

			default:
				assert(false && "invalid AR calculation method");
			}

			sorting.method = attackPowerMethod;

			e.Skip(true);
		}
	};

	auto GetLevels()
	{
		wxArrayString levels;
		for (int i = 0; i <= 10; ++i)
			levels.Add(fmt::format("+{}", i));
		return levels;
	}

	Finder* const finder;

	wxSearchCtrl* filter;
	wxButton* openTypeFilter;
	wxButton* openInfusionFilter;
	wxButton* openSorting;
	wxChoice* levels;

	TypeFilter* typeFilter;
	InfusionFilter* infusionFilter;
	SortingFilter* sortingFilter;

	std::string filterText;
	std::set<Type> types{};
	std::set<Infusion> infusions{Infusion::None};
	Sorting sorting{};

public:
	FilterControls(wxWindow* parent, Finder* finder)
		: wxPanel(parent)
		, finder(finder)
		, filter(new wxSearchCtrl(this, wxID_ANY))
		, openTypeFilter(new wxButton(this, wxID_ANY, wxT("Classes")))
		, openInfusionFilter(new wxButton(this, wxID_ANY, wxT("Infusions")))
		, openSorting(new wxButton(this, wxID_ANY, wxT("Sorting")))
		, levels(new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, GetLevels()))
		, typeFilter(new TypeFilter(this))
		, infusionFilter(new InfusionFilter(this))
		, sortingFilter(new SortingFilter(this))
	{
		for (int type = 0; type < static_cast<int>(Type::Size); ++type)
			types.emplace(static_cast<Type>(type));

		assert(levels->GetCount() == 11 && "invalid level count");
		levels->SetSelection(10);

		this->SetMaxSize(wxSize{128 * 5, 99999});

		filter->SetFocus();
		filter->SetDescriptiveText(wxT("Filter..."));
		filter->Bind(wxEVT_TEXT, &FilterControls::OnFilter, this);

		openTypeFilter->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { this->ShowPopup(typeFilter, openTypeFilter); });
		typeFilter->Bind(wxEVT_CHECKBOX, &FilterControls::OnType, this);

		openInfusionFilter->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { this->ShowPopup(infusionFilter, openInfusionFilter); });
		infusionFilter->Bind(wxEVT_CHECKBOX, &FilterControls::OnInfusion, this);

		openSorting->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) { this->ShowPopup(sortingFilter, openSorting); });
		sortingFilter->Bind(wxEVT_CHECKBOX, &FilterControls::OnSorting, this);
		sortingFilter->Bind(wxEVT_RADIOBOX, &FilterControls::OnSorting, this);

		levels->Bind(wxEVT_CHOICE, &FilterControls::OnLevel, this);

		auto* sizer = new wxBoxSizer(wxHORIZONTAL);

		sizer->Add(filter, 6, wxALIGN_CENTRE_VERTICAL);
		sizer->AddStretchSpacer(1);
		sizer->Add(openTypeFilter, 4, wxALIGN_CENTRE_VERTICAL);
		sizer->Add(openInfusionFilter, 4, wxALIGN_CENTRE_VERTICAL);
		sizer->AddStretchSpacer(1);
		sizer->Add(openSorting, 4, wxALIGN_CENTRE_VERTICAL);
		sizer->AddStretchSpacer(1);
		sizer->Add(levels, 1, wxALIGN_CENTRE_VERTICAL);

		this->SetSizer(sizer);
	}

	const auto& GetWeaponFilter() const
	{
		return filterText;
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

	void OnFilter(wxCommandEvent&)
	{
		filterText = filter->GetValue();
		finder->OnFilterControlsUpdate();
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

	void OnSorting(wxCommandEvent&)
	{
		sorting = sortingFilter->GetWeaponSorting();
		finder->OnFilterControlsUpdate();
	}

	void OnLevel(wxCommandEvent&)
	{
		finder->grid->SetAllLevel(levels->GetSelection(), sorting);
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
	sorting = controls->GetWeaponSorting();
	grid->SetFiltering(controls->GetWeaponFilter(), controls->GetWeaponTypes(), controls->GetWeaponInfusions(), sorting);
}
