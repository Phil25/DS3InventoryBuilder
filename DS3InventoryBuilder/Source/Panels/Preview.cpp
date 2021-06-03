#include "Preview.h"

#include <AppMain.h>
#include <Context/IAttributesListener.h>
#include <Context/ISelectionListener.h>
#include <Calculator.h>
#include <wx/notebook.h>
#include <wx/dataview.h>
#include <format>

namespace
{
	inline bool ToFloat(const char* str, float& out) noexcept
	{
		char* end = nullptr;
		out = std::strtof(str, &end);
		return *end == 0; // success
	}

	inline auto ToString(const float val) -> std::string
	{
		return std::format("{:.2f}", val);
	}

	inline auto ToString(const bool val) -> std::string
	{
		return val ? "Yes" : "No";
	}

	inline auto ToString(const invbuilder::Weapon::Type type) -> std::string
	{
		using T = invbuilder::Weapon::Type;
		switch (type)
		{
		case T::Dagger: return "Dagger";
		case T::StraightSword: return "Straight Sword";
		case T::Greatsword: return "Greatsword";
		case T::UltraGreatsword: return "Ultra Greatsword";
		case T::CurvedSword: return "Curved Sword";
		case T::CurvedGreatsword: return "Curved Greatsword";
		case T::ThrustingSword: return "Thrusting Sword";
		case T::Katana: return "Katana";
		case T::Axe: return "Axe";
		case T::Greataxe: return "Greataxe";
		case T::Hammer: return "Hammer";
		case T::GreatHammer: return "Great Hammer";
		case T::Spear: return "Spear";
		case T::Pike: return "Pike";
		case T::Halberd: return "Halberd";
		case T::Reaper: return "Reaper";
		case T::Whip: return "Whip";
		case T::Fist: return "Fist";
		case T::Claw: return "Claw";
		case T::Bow: return "Bow";
		case T::Greatbow: return "Greatbow";
		case T::Crossbow: return "Crossbow";
		case T::Staff: return "Staff";
		case T::PyromancyFlame: return "Pyromancy Flame";
		case T::Talisman: return "Talisman";
		case T::SacredChime: return "Sacred Chime";
		case T::Torch: return "Torch";
		case T::SmallShield: return "Small Shield";
		case T::Shield: return "Shield";
		case T::Greatshield: return "Greatshield";
		}

		assert(false && "invalid weapon class");
		return "ERROR";
	}

	inline auto GetDisplayName(const std::string& name, const bool unique, const int level, const invbuilder::Weapon::Infusion infusion)
	{
		using DB = invbuilder::Database;
		using Infusion = invbuilder::Weapon::Infusion;

		return infusion == Infusion::None
			? std::format("{} +{}", name, DB::GetDisplayLevel(unique, level))
			: std::format("{} {} +{}", name, DB::ToString(infusion), DB::GetDisplayLevel(unique, level));
	}
}

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

	void OnUpdate(const int) override
	{
		preview->OnSelectionUpdate();
	}
};

class Preview::TextHeader final : public wxStaticText
{
public:
	TextHeader(wxWindow* parent, const char* text, int size=16) : wxStaticText(parent, wxID_ANY, text)
	{
		this->SetFont(wxFont{size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD});
	}
};

class Preview::PreviewIcon final : public wxPanel
{
	static const int size = 128;
	std::vector<std::string> weapons;

public:
	PreviewIcon(wxWindow* parent) : wxPanel(parent)
	{
		this->SetMinSize(wxSize{size, size});
		this->Bind(wxEVT_PAINT, &PreviewIcon::OnRender, this);
	}

	void ClearWeaponIcon()
	{
		weapons.clear();
		Refresh();
	}

	void SetWeaponIcon(std::string name)
	{
		weapons.clear();
		weapons.emplace_back(std::move(name));
		Refresh();
	}

	void SetWeaponIcons(std::vector<std::string> weapons)
	{
		this->weapons = std::move(weapons);
		Refresh();
	}

private:
	void OnRender(wxPaintEvent&)
	{
		if (weapons.empty())
			return;

		auto dc = wxPaintDC{this};

		const auto weaponCount = std::min(weapons.size(), 25ULL);
		const auto rowCount = GetRowCount(weaponCount);
		const auto weaponSize = size / rowCount;

		for (int i = 0; i < weaponCount; ++i)
		{
			const auto col = i % rowCount;
			const auto row = i / rowCount;
			dc.DrawBitmap(wxGetApp().GetImage(weapons[i], weaponSize), col * weaponSize, row * weaponSize, false);
		}
	}

	int GetRowCount(const size_t weaponsCount)
	{
		if (weaponsCount <= 1) return 1;
		if (weaponsCount <= 4) return 2;
		if (weaponsCount <= 9) return 3;
		if (weaponsCount <= 16) return 4;
		return 5; // if (weaponsCount <= 25)
	}
};

class Section final : public wxPanel
{
	static const auto height = 240;

	class DataListWrapper : public wxPanel
	{
		// adjusted to fit "Lightning Sellsword Twinblades +10"
		static const auto defaultNameColumnWidth = 196U;
		static const auto minValueColumnWidth = 40U;

		class Model : public wxDataViewListStore
		{
			int Compare(const wxDataViewItem& item1, const wxDataViewItem& item2, unsigned int column, bool ascending) const override
			{
				wxVariant var1, var2;
				GetValue(var1, item1, column);
				GetValue(var2, item2, column);

				const auto& str1 = var1.GetString();
				const auto& str2 = var2.GetString();

				float val1, val2;
				const bool isNum = ToFloat(str1.c_str(), val1) * ToFloat(str2.c_str(), val2);

				const auto compared = isNum ? val1 > val2 : str1 > str2;
				return ascending == compared ? 1 : -1;
			}
		};

		wxObjectDataPtr<Model> model;
		wxDataViewListCtrl* view;

	public:
		DataListWrapper(wxWindow* parent, const std::vector<std::string>& columns)
			: wxPanel(parent, wxID_ANY)
			, model(new Model)
			, view(new wxDataViewListCtrl{this, wxID_ANY})
		{
			view->AssociateModel(model.get());

			this->AddColumn(columns[0], defaultNameColumnWidth);
			for (int i = 1; i < columns.size(); ++i)
				this->AddColumn(columns[i], GetValueColumnWidth(columns[i]));

			auto* sizer = new wxBoxSizer(wxVERTICAL);
			sizer->Add(view, 1, wxEXPAND, 0);
			this->SetSizer(sizer);
		}

		void AddWeapon(std::vector<std::string> values)
		{
			wxVector<wxVariant> data;
			for (auto& value : values)
				data.push_back(std::move(value));
			view->AppendItem(data);
		}

		void ClearWeapons()
		{
			view->DeleteAllItems();
		}

	private:
		inline auto GetValueColumnWidth(const std::string& name) -> decltype(minValueColumnWidth)
		{
			const auto length = static_cast<decltype(minValueColumnWidth)>(name.length());
			return std::max(minValueColumnWidth + length * 3, minValueColumnWidth);
		}

		inline void AddColumn(const std::string& name, unsigned int width)
		{
			view->AppendTextColumn(name, wxDATAVIEW_CELL_INERT, width, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_SORTABLE);
		}
	};

	Preview::TextHeader* header;
	DataListWrapper* list;

public:
	Section(wxWindow* parent, const char* name, const std::vector<std::string>& columns)
		: wxPanel(parent, wxID_ANY)
		, header(new Preview::TextHeader{this, name, 10})
		, list(new DataListWrapper{this, columns})
	{
		this->SetMinSize(wxSize{0, height});
		this->SetMaxSize(wxSize{99999, height});

		auto* sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(header, 0);
		sizer->Add(list, 1, wxEXPAND, 0);
		this->SetSizer(sizer);
	}

	void AddWeapon(std::vector<std::string> values)
	{
		list->AddWeapon(std::move(values));
	}

	void ClearWeapons()
	{
		list->ClearWeapons();
	}
};

class Preview::WeaponBook final : public wxNotebook
{
	class PageOffensive final : public wxScrolledWindow
	{
		Section* attackRating;
		Section* statusEffects;

	public:
		PageOffensive(wxWindow* parent)
			: wxScrolledWindow(parent)
			, attackRating(new Section{this, "Attack Rating", {"Weapon", "Total", "Physical", "Magic", "Fire", "Lightning", "Dark"}})
			, statusEffects(new Section{this, "Status Effects", {"Weapon", "Bleed", "Poison", "Frost"}})
		{
			auto* sizer = new wxBoxSizer(wxVERTICAL);

			sizer->Add(attackRating, 1, wxEXPAND | wxALL, 5);
			sizer->Add(statusEffects, 1, wxEXPAND | wxALL, 5);

			this->SetScrollRate(20, 20);
			this->SetSizer(sizer);
		}

		void ClearWeapons()
		{
			attackRating->ClearWeapons();
			statusEffects->ClearWeapons();
		}

		void AddWeapon(const invbuilder::Weapon& weapon, const int level, const invbuilder::Weapon::Infusion infusion)
		{
			// TODO: allow to pass const invbuilder::Weapon& instead of db and name cstr
			const auto attribs = wxGetApp().GetSessionData().GetAttributes();
			const auto& [attack, status] = invbuilder::calculator::AttackRating(
				wxGetApp().GetDatabase(), weapon.name.c_str(),
				infusion, level, attribs);
			std::string name = GetDisplayName(weapon.name, weapon.unique, level, infusion);

			attackRating->AddWeapon({name,
				ToString(attack.Total()),
				ToString(attack.physical),
				ToString(attack.magic),
				ToString(attack.fire),
				ToString(attack.lightning),
				ToString(attack.dark),
			});

			statusEffects->AddWeapon({name,
				ToString(status.bleed),
				ToString(status.poison),
				ToString(status.frost)
			});
		}
	};

	class PageDefensive final : public wxScrolledWindow
	{
		Section* absorptions;
		Section* resistances;

	public:
		PageDefensive(wxWindow* parent)
			: wxScrolledWindow(parent)
			, absorptions(new Section{this, "Absorptions", {"Weapon", "Stability", "Physical", "Magic", "Fire", "Lightning", "Dark"}})
			, resistances(new Section{this, "Resistances", {"Weapon", "Bleed", "Poison", "Frost", "Curse"}})
		{
			auto* sizer = new wxBoxSizer(wxVERTICAL);

			sizer->Add(absorptions, 1, wxEXPAND | wxALL, 5);
			sizer->Add(resistances, 1, wxEXPAND | wxALL, 5);

			this->SetScrollRate(20, 20);
			this->SetSizer(sizer);
		}

		void ClearWeapons()
		{
			absorptions->ClearWeapons();
			resistances->ClearWeapons();
		}

		void AddWeapon(const invbuilder::Weapon& weapon, const int level, const invbuilder::Weapon::Infusion infusion)
		{
			assert(!(infusion != invbuilder::Weapon::Infusion::None && !weapon.infusable) && "accessing infusion on non-infusable weapon");
			const auto& data = weapon.properties.at(infusion).level[level];
			std::string name = GetDisplayName(weapon.name, weapon.unique, level, infusion);

			absorptions->AddWeapon({name,
				ToString(data.stability),
				ToString(data.absorption.physical),
				ToString(data.absorption.magic),
				ToString(data.absorption.fire),
				ToString(data.absorption.lightning),
				ToString(data.absorption.dark),
			});

			resistances->AddWeapon({name,
				ToString(data.resistance.bleed),
				ToString(data.resistance.poison),
				ToString(data.resistance.frost),
				ToString(data.resistance.curse),
			});
		}
	};

	class PageProperties final : public wxScrolledWindow
	{
		Section* generic;
		Section* scaling;
		Section* requirements;

	public:
		PageProperties(wxWindow* parent)
			: wxScrolledWindow(parent)
			, generic(new Section{this, "Generic", {"Weapon", "Class", "Weight", "Infusable", "Buffable"}})
			, scaling(new Section{this, "Scaling", {"Weapon", "STR", "DEX", "INT", "FTH", "LCK"}})
			, requirements(new Section{this, "Requirements", {"Weapon", "STR", "DEX", "INT", "FTH"}})
		{
			auto* sizer = new wxBoxSizer(wxVERTICAL);

			sizer->Add(generic, 1, wxEXPAND | wxALL, 5);
			sizer->Add(scaling, 1, wxEXPAND | wxALL, 5);
			sizer->Add(requirements, 1, wxEXPAND | wxALL, 5);

			this->SetScrollRate(20, 20);
			this->SetSizer(sizer);
		}

		void ClearWeapons()
		{
			generic->ClearWeapons();
			scaling->ClearWeapons();
			requirements->ClearWeapons();
		}

		void AddWeapon(const invbuilder::Weapon& weapon, const int level, const invbuilder::Weapon::Infusion infusion)
		{
			assert(!(infusion != invbuilder::Weapon::Infusion::None && !weapon.infusable) && "accessing infusion on non-infusable weapon");
			const auto& scl = weapon.properties.at(infusion).level[level].scaling;
			std::string name = GetDisplayName(weapon.name, weapon.unique, level, infusion);

			generic->AddWeapon({name,
				ToString(weapon.type),
				ToString(weapon.weight),
				ToString(weapon.infusable),
				ToString(weapon.buffable),
			});

			scaling->AddWeapon({name,
				ToString(scl.strength),
				ToString(scl.dexterity),
				ToString(scl.intelligence),
				ToString(scl.faith),
				ToString(scl.luck),
			});

			requirements->AddWeapon({name,
				std::to_string(std::lround(weapon.requirements.strength)),
				std::to_string(std::lround(weapon.requirements.dexterity)),
				std::to_string(std::lround(weapon.requirements.intelligence)),
				std::to_string(std::lround(weapon.requirements.faith))
			});
		}
	};

	class PageMoveset final : public wxWindow
	{
	public:
		PageMoveset(wxWindow* parent) : wxWindow(parent, wxID_ANY)
		{
			auto* sizer = new wxBoxSizer(wxVERTICAL);
			this->SetSizer(sizer);
		}
	};

	class PageRanges final : public wxWindow
	{
	public:
		PageRanges(wxWindow* parent) : wxWindow(parent, wxID_ANY)
		{
			auto* sizer = new wxBoxSizer(wxVERTICAL);
			this->SetSizer(sizer);
		}
	};

	class PageCombos final : public wxWindow
	{
	public:
		PageCombos(wxWindow* parent) : wxWindow(parent, wxID_ANY)
		{
			auto* sizer = new wxBoxSizer(wxVERTICAL);
			this->SetSizer(sizer);
		}
	};

	class PageRemarks final : public wxWindow
	{
	public:
		PageRemarks(wxWindow* parent) : wxWindow(parent, wxID_ANY)
		{
			auto* sizer = new wxBoxSizer(wxVERTICAL);
			this->SetSizer(sizer);
		}
	};

	PageOffensive* pageOffensive{nullptr};
	PageDefensive* pageDefensive{nullptr};
	PageProperties* pageProperties{nullptr};
	PageMoveset* pageMoveset{nullptr};
	PageRanges* pageRanges{nullptr};
	PageCombos* pageCombos{nullptr};
	PageRemarks* pageRemarks{nullptr};

public:
	WeaponBook(wxPanel* parent)
		: wxNotebook(parent, wxID_ANY)
		, pageOffensive(new PageOffensive{this})
		, pageDefensive(new PageDefensive{this})
		, pageProperties(new PageProperties{this})
		, pageMoveset(new PageMoveset{this})
		, pageRanges(new PageRanges{this})
		, pageCombos(new PageCombos{this})
		, pageRemarks(new PageRemarks{this})
	{
		this->AddPage(pageOffensive, "Offensive", true);
		this->AddPage(pageDefensive, "Defensive");
		this->AddPage(pageProperties, "Properties");
		this->AddPage(pageMoveset, "Moveset");
		this->AddPage(pageRanges, "Ranges");
		this->AddPage(pageCombos, "Combos");
		this->AddPage(pageRemarks, "Remarks");
	}

	void UpdateSelection(const std::vector<std::shared_ptr<WeaponContext>>& selection)
	{
		pageOffensive->ClearWeapons();
		pageDefensive->ClearWeapons();
		pageProperties->ClearWeapons();

		for (const auto& weaponContext : selection)
		{
			const auto& weapon = wxGetApp().GetDatabase().GetWeapon(weaponContext->GetName());
			pageProperties->AddWeapon(weapon, weaponContext->GetLevel(), weaponContext->GetInfusion());
			pageOffensive->AddWeapon(weapon, weaponContext->GetLevel(), weaponContext->GetInfusion());
			pageDefensive->AddWeapon(weapon, weaponContext->GetLevel(), weaponContext->GetInfusion());
		}
	}
};

Preview::Preview(wxWindow* parent)
	: Title(parent, "Weapon Preview")
	, attributesListener(std::make_shared<AttributesListener>(this))
	, selectionListener(std::make_shared<SelectionListener>(this))
	, label(new TextHeader{GetContent(), "No weapons selected"})
	, icon(new PreviewIcon{GetContent()})
	, book(new WeaponBook{GetContent()})
{
	attributesListener->Register();
	selectionListener->Register();

	auto* top = new wxBoxSizer(wxHORIZONTAL);
	top->Add(label);
	top->AddStretchSpacer(1);
	top->Add(icon);

	auto* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(top, 0, wxEXPAND | wxALL, 3);
	sizer->Add(book, 1, wxEXPAND | wxALL, 3);

	GetContent()->SetSizer(sizer);
}

void Preview::OnSelectionUpdate()
{
	const auto& weakSelection = wxGetApp().GetSessionData().GetSelection();

	std::vector<std::shared_ptr<WeaponContext>> selection;
	selection.reserve(weakSelection.size());

	for (const auto& weaponContext : weakSelection)
		if (const auto ptr = weaponContext.lock(); ptr)
			selection.emplace_back(ptr);
		else
			assert(false && "weapon context should be valid at this point");

	if (!selection.size())
	{
		label->SetLabel("No weapon selected");
		icon->ClearWeaponIcon();
	}
	else if (selection.size() == 1)
	{
		label->SetLabel(selection[0]->GetName());
		icon->SetWeaponIcon(selection[0]->GetName());
	}
	else
	{
		label->SetLabel(std::format("{}x weapons selected", selection.size()));

		std::vector<std::string> weapons;
		for (const auto& weapon : selection)
			weapons.push_back(weapon->GetName());

		icon->SetWeaponIcons(std::move(weapons));
	}

	book->UpdateSelection(selection);
}
