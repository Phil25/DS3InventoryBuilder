#include "Preview.h"

#include <AppMain.h>
#include <Context/IAttributesListener.h>
#include <Context/ISelectionListener.h>
#include <Calculator.h>
#include <wx/notebook.h>
#include <wx/dataview.h>
#include <wx/listctrl.h>
#include <wx/clipbrd.h>
#include <fmt/core.h>
#include <sstream>

namespace
{
	inline bool ToFloat(const char* str, float& out) noexcept
	{
		char* end = nullptr;
		out = std::strtof(str, &end);
		return *end == 0; // success
	}

	inline auto ToString(const float val, const char* whenZero="0") -> std::string
	{
		if (!val) return whenZero;
		return fmt::format("{:.2f}", val);
	}

	inline auto ToString1AfterComma(const float val, const char* whenZero="0") -> std::string
	{
		if (!val) return whenZero;
		return fmt::format("{:.1f}", val);
	}

	inline auto ToStringInt(const float val, const char* whenZero="0") -> std::string
	{
		if (!val) return whenZero;
		return std::to_string(static_cast<int>(val));
	}

	inline auto ToStringIntPair(const float val1, const float val2, const char* whenZero="0") -> std::string
	{
		if (!val2 || (std::abs(val1 - val2) < 0.1))
			return ToStringInt(val1, whenZero);
		else
			return fmt::format("{} ({})", ToStringInt(val1, whenZero), ToStringInt(val2, whenZero));
	}

	inline auto ToString(const bool val) -> std::string
	{
		return val ? "Yes" : "No";
	}

	inline auto GetDisplayName(const std::string& name, const bool unique, const int level, const invbuilder::Weapon::Infusion infusion)
	{
		using DB = invbuilder::Database;
		using Infusion = invbuilder::Weapon::Infusion;

		return infusion == Infusion::None
			? fmt::format("{} +{}", name, DB::GetDisplayLevel(unique, level))
			: fmt::format("{} {} +{}", name, DB::ToString(infusion), DB::GetDisplayLevel(unique, level));
	}

	inline auto CreateMassItemGibCode(const std::shared_ptr<WeaponContext>& context)
	{
		using DB = invbuilder::Database;

		const auto infusion = DB::GetInGameInfusionID(context->GetInfusion());
		const auto level = DB::GetDisplayLevel(context->IsUnique(), context->GetLevel());

		return fmt::format("{:X},{},{}", context->GetID(), infusion, level);
	}

	inline auto CreateMassItemGibCode(const WeaponContext::Vector& selection)
	{
		if (!selection.size())
			return std::string{};

		std::ostringstream oss;
		for (const auto& context : selection)
			oss << CreateMassItemGibCode(context) << '\n';

		auto str = oss.str();
		str.pop_back(); // remove last newline

		return str;
	}

	const auto itemGibMessage = wxString{
		"Download the latest The Grand Archives CE table?"
		"\n\n"
		"This will allow you to inject selected weapons into DS3 using Cheat Engine."
		"\n\n"
		"Mass ItemGib from the Dark-Souls-III-CT-TGA table can spawn multiple weapons at once. "
		"To use it, load into your character, open Cheat Engine, select DarkSoulsIII.exe and load the table."
		"\n\n"
		"Once loaded, navigate to:\n"
		"Scripts > Build Creation > ItemGib > Mass ItemGib > Select weapons"
		"\n\n"
		"Paste the Mass ItemGib code and click \"Spawn Weapons\" to have them added to your inventory. "
		"Using the \"Discard Selected\" option will allow you to select every weapon, bow and casting tool then discard them in one go. "
		"This way you can swap inventories with just a few clicks. "
		"Every ItemGib feature is safe."
		"\n\n"
		"Clicking \"OK\" will open the releases page where you can download the table."
	};
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

	void OnUpdate(const GridRole) override
	{
		preview->OnSelectionUpdate();
	}
};

class Preview::TextHeader final : public wxStaticText
{
public:
	TextHeader(wxWindow* parent, const char* text, const int size=16, const wxFontWeight weight=wxFONTWEIGHT_BOLD)
		: wxStaticText(parent, wxID_ANY, text)
	{
		this->SetFont(wxFont{size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, weight});
	}
};

class Preview::WeaponLabel final : public wxPanel
{
	using DB = invbuilder::Database;
	using Infusion = invbuilder::Weapon::Infusion;

	class MassItemGibPanel final : public wxPanel
	{
		TextHeader* label;
		wxTextCtrl* code;

		wxBoxSizer* sizerButtons;
		wxBoxSizer* sizerCode;

	public:
		MassItemGibPanel(wxWindow* parent)
			: wxPanel(parent)
			, label(new TextHeader{this, "Mass ItemGib code:", 14, wxFONTWEIGHT_NORMAL})
			, code(new wxTextCtrl{this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY})
			, sizerButtons(new wxBoxSizer{wxVERTICAL})
			, sizerCode(new wxBoxSizer{wxVERTICAL})
		{
			auto* copy = new wxButton{this, wxID_ANY, wxT("Copy")};
			auto* help = new wxButton{this, wxID_ANY, wxT("HowTo")};

			copy->Bind(wxEVT_BUTTON, &MassItemGibPanel::OnCopy, this);
			help->Bind(wxEVT_BUTTON, &MassItemGibPanel::OnHelp, this);

			sizerButtons->Add(copy, 1, wxEXPAND);
			sizerButtons->Add(help, 1, wxEXPAND);

			sizerCode->Add(label, 1, wxEXPAND);
			sizerCode->Add(code, 1, wxEXPAND);
			sizerCode->Hide(label);

			auto* sizer = new wxBoxSizer{wxHORIZONTAL};
			sizer->Add(sizerButtons, 0, wxEXPAND | wxRIGHT, 3);
			sizer->Add(sizerCode, 1, wxEXPAND);

			this->SetSizer(sizer);
		}

		void SetSingle()
		{
			sizerCode->Show(label);
			Layout();
		}

		void SetMultiple()
		{
			sizerCode->Hide(label);
			Layout();
		}

		void SetMassItemGibCode(std::string val)
		{
			code->SetLabel(std::move(val));
		}

	private:
		void OnCopy(wxCommandEvent&)
		{
			if (!wxTheClipboard->Open())
			{
				wxMessageDialog{nullptr, wxT("Failure accessing the system clipboard."), wxT("Copy Mass ItemGib code"), wxOK | wxICON_ERROR}.ShowModal();
				return;
			}

			wxTheClipboard->SetData(new wxTextDataObject(code->GetLabel()));
			wxTheClipboard->Flush();
			wxTheClipboard->Close();
		}

		void OnHelp(wxCommandEvent&)
		{
			auto dialog = wxMessageDialog{nullptr, itemGibMessage, wxT("Mass ItemGib code"), wxOK | wxCANCEL};

			if (dialog.ShowModal() == wxID_OK)
				wxLaunchDefaultBrowser("https://github.com/inunorii/Dark-Souls-III-CT-TGA/releases/latest");
		}
	};

	wxBoxSizer* sizerMain;
	wxBoxSizer* sizerSubtitle;

	TextHeader* title;
	TextHeader* subtitle;
	wxPanel* infusionIcon;
	std::string infusionName;
	MassItemGibPanel* itemGibPanel;

public:
	WeaponLabel(wxWindow* parent)
		: wxPanel(parent)
		, sizerMain(new wxBoxSizer{wxVERTICAL})
		, sizerSubtitle(new wxBoxSizer{wxHORIZONTAL})
		, title(new TextHeader{this, "No weapons selected"})
		, subtitle(new TextHeader{this, "", 14, wxFONTWEIGHT_NORMAL})
		, infusionIcon(new wxPanel{this})
		, itemGibPanel(new MassItemGibPanel{this})
	{
		infusionIcon->SetMinSize(wxSize{30, 30});
		infusionIcon->Bind(wxEVT_PAINT, &WeaponLabel::OnInfusionPaint, this);

		sizerSubtitle->Add(infusionIcon, 0, wxALIGN_CENTER_VERTICAL | wxALL, 3);
		sizerSubtitle->Add(subtitle, 0, wxALIGN_CENTER_VERTICAL | wxALL, 3);

		sizerMain->Add(title, 0, wxEXPAND | wxALL, 3);
		sizerMain->Add(sizerSubtitle, 0, wxEXPAND | wxALL, 3);
		sizerMain->Add(itemGibPanel, 1, wxEXPAND | wxALL, 3);

		sizerMain->Hide(itemGibPanel);

		this->SetSizer(sizerMain);
	}

	void SetNoWeapons()
	{
		title->SetLabel("No weapon selected");
		subtitle->SetLabel("");

		sizerSubtitle->Hide(infusionIcon);
		sizerMain->Hide(itemGibPanel);

		Layout();
	}

	void SetWeapon(const std::shared_ptr<WeaponContext>& context)
	{
		const auto& weapon = wxGetApp().GetDatabase().GetWeapon(context->GetName());

		std::string infusionText = weapon.infusable ? "Not infused" : "Uninfusable";

		sizerSubtitle->Hide(infusionIcon);

		itemGibPanel->SetSingle();
		itemGibPanel->SetMassItemGibCode(CreateMassItemGibCode(context));
		sizerMain->Show(itemGibPanel);

		const auto inf = context->GetInfusion();

		if (inf != Infusion::None)
		{
			infusionName = DB::ToString(context->GetInfusion());
			infusionText = infusionName;

			sizerSubtitle->Show(infusionIcon);
			infusionIcon->Refresh();
		}

		title->SetLabel(context->GetName());
		subtitle->SetLabel(fmt::format("{} {} +{}", infusionText, DB::ToString(weapon.type), context->GetLevel(true)));

		Layout();
	}

	void SetWeapons(const WeaponContext::Vector& selection)
	{
		title->SetLabel(fmt::format("{}x weapons selected", selection.size()));
		subtitle->SetLabel("Mass ItemGib code:");

		sizerSubtitle->Hide(infusionIcon);

		itemGibPanel->SetMultiple();
		itemGibPanel->SetMassItemGibCode(CreateMassItemGibCode(selection));
		sizerMain->Show(itemGibPanel);

		Layout();
	}

private:
	void OnInfusionPaint(wxPaintEvent&)
	{
		if (!infusionName.empty() && infusionName != "None")
			wxPaintDC{infusionIcon}.DrawBitmap(wxGetApp().GetImage(infusionName, 30), 0, 0, false);
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

		const auto weaponCount = std::min(weapons.size(), static_cast<size_t>(25));
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
			return std::max(minValueColumnWidth + length * 3, minValueColumnWidth * 1); // force pass by copy
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

class Preview::WeaponSimple final : public wxScrolledWindow
{
	class PropertyList final : public wxListView
	{
	public:
		PropertyList(wxWindow* parent, const std::vector<wxString>& rows)
			: wxListView(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_NO_HEADER | wxLC_HRULES)
		{
			AppendColumn("Property");
			AppendColumn("Value");

			int id = 0;
			for (const auto& row : rows)
			{
				InsertItem(id, row);
				SetItem(id, 1, "-");
				++id;
			}

			auto font = this->GetFont().MakeLarger().MakeLarger();
			this->SetFont(font);

			SetItemFont(0, font.MakeBold());
			SetItemTextColour(2, wxColor{66,139,202}); // magic
			SetItemTextColour(3, wxColor{217,83,79}); // fire
			SetItemTextColour(4, wxColor{255,191,0}); // lightning
			SetItemTextColour(5, wxColor{85,85,85}); // dark
			SetItemTextColour(6, wxColor{255,0,0}); // bleed
			SetItemTextColour(7, wxColor{92,184,92}); // poison
			SetItemTextColour(8, wxColor{91,192,222}); // frost

			SetMinSize(wxSize{10, 280}); // originally too large width
			SetColumnWidth(0, 120);
			SetColumnWidth(1, 120);
		}
	};

	class Value final : public wxPanel
	{
		wxStaticText* label;
		wxStaticText* value;
		wxColour defaultForegroundColor;

	public:
		Value(wxWindow* parent, wxString labelText, wxString valueText=wxT("-"))
			: wxPanel(parent)
			, label(new wxStaticText(this, wxID_ANY, std::move(labelText), wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT))
			, value(new wxStaticText(this, wxID_ANY, std::move(valueText), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT))
			, defaultForegroundColor(label->GetForegroundColour())
		{
			label->SetFont(wxFont{11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD});
			value->SetFont(wxFont{11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_MEDIUM});

			auto* sizer = new wxBoxSizer(wxHORIZONTAL);
			sizer->Add(label, 1, wxEXPAND | wxALL, 3);
			sizer->Add(value, 1, wxEXPAND | wxALL, 3);
			this->SetSizer(sizer);
		}

		void SetLabelValue(wxString valueText)
		{
			value->SetLabelText(std::move(valueText));
		}

		void SetColorWarning(int degree=0)
		{
			auto color =
				degree == 2 ? wxColor{204,51,0} :
				degree == 1 ? wxColor{211,166,37} : defaultForegroundColor;

			label->SetForegroundColour(color);
			value->SetForegroundColour(color);

			Update();
			Refresh();
		}
	};

	class Properties final : public wxPanel
	{
		Value* weight;
		Value* buffable;
		Value* infusable;

	public:
		Properties(wxWindow* parent)
			: wxPanel(parent)
			, weight(new Value(this, wxT("Weight")))
			, buffable(new Value(this, wxT("Buffable")))
			, infusable(new Value(this,wxT("Infusable")))
		{
			auto* sizer = new wxBoxSizer(wxVERTICAL);

			auto* sizerProps = new wxBoxSizer(wxHORIZONTAL);
			sizerProps->Add(weight, 1, wxEXPAND);
			sizerProps->Add(buffable, 1, wxEXPAND);
			sizerProps->Add(infusable, 1, wxEXPAND);

			sizer->Add(new Preview::TextHeader(this, "Properties", 12), 0, wxEXPAND | wxALL, 3);
			sizer->Add(sizerProps, 0, wxEXPAND | wxALL, 3);

			this->SetSizer(sizer);
		}

		void UpdateWeapon(const invbuilder::Weapon& weapon)
		{
			weight->SetLabelValue(ToString1AfterComma(weapon.weight));
			buffable->SetLabelValue(ToString(weapon.buffable));
			infusable->SetLabelValue(ToString(weapon.infusable));
		}
	};

	class AttackRating final : public wxPanel
	{
		Preview::TextHeader* header;
		PropertyList* list;

	public:
		AttackRating(wxWindow* parent)
			: wxPanel(parent)
			, header(new Preview::TextHeader(this, "Attack Rating (2H)", 12))
			, list(new PropertyList(this, {"Total", "Physical", "Magic", "Fire", "Lightning", "Dark", "Bleed", "Poison", "Frost"}))
		{
			auto* sizer = new wxBoxSizer(wxVERTICAL);

			sizer->Add(header, 0, wxEXPAND);
			sizer->Add(list, 0, wxEXPAND);

			this->SetSizer(sizer);
		}

		void UpdateWeapon(const invbuilder::Weapon& weapon, const int level, const invbuilder::Weapon::Infusion infusion)
		{
			const auto attribs = wxGetApp().GetSessionData().GetAttributes();
			const auto& [damage, status] = invbuilder::calculator::AttackRating(
				wxGetApp().GetDatabase(), weapon.name.c_str(), infusion, level, attribs, false);

			const auto& [damage2h, status2h] = invbuilder::calculator::AttackRating(
				wxGetApp().GetDatabase(), weapon.name.c_str(), infusion, level, attribs, true);

			list->SetItem(0, 1, ToStringIntPair(damage.Total(), damage2h.Total(), "-"));
			list->SetItem(1, 1, ToStringIntPair(damage.physical, damage2h.physical, "-"));
			list->SetItem(2, 1, ToStringIntPair(damage.magic, damage2h.magic, "-"));
			list->SetItem(3, 1, ToStringIntPair(damage.fire, damage2h.fire, "-"));
			list->SetItem(4, 1, ToStringInt(damage.lightning, "-"));
			list->SetItem(5, 1, ToStringInt(damage.dark, "-"));
			list->SetItem(6, 1, ToStringInt(status.bleed, "-"));
			list->SetItem(7, 1, ToStringInt(status.poison, "-"));
			list->SetItem(8, 1, ToStringInt(status.frost, "-"));
		}
	};

	class GuardAbsorption final : public wxPanel
	{
		Preview::TextHeader* header;
		PropertyList* list;

	public:
		GuardAbsorption(wxWindow* parent)
			: wxPanel(parent)
			, header(new Preview::TextHeader(this, "Guard Absorption", 12))
			, list(new PropertyList(this, {"Stability", "Physical", "Magic", "Fire", "Lightning", "Dark", "Bleed", "Poison", "Frost", "Curse"}))
		{
			auto* sizer = new wxBoxSizer(wxVERTICAL);

			sizer->Add(header, 0, wxEXPAND);
			sizer->Add(list, 0, wxEXPAND);

			this->SetSizer(sizer);
		}

		void UpdateWeapon(const invbuilder::Weapon& weapon, const int level, const invbuilder::Weapon::Infusion infusion)
		{
			const auto& stability = weapon.properties.at(infusion).level[level].stability;
			const auto& absorption = weapon.properties.at(infusion).level[level].absorption;
			const auto& resistance = weapon.properties.at(infusion).level[level].resistance;

			list->SetItem(0, 1, ToStringInt(stability, "-"));
			list->SetItem(1, 1, ToString1AfterComma(absorption.physical, "-"));
			list->SetItem(2, 1, ToString1AfterComma(absorption.magic, "-"));
			list->SetItem(3, 1, ToString1AfterComma(absorption.fire, "-"));
			list->SetItem(4, 1, ToString1AfterComma(absorption.lightning, "-"));
			list->SetItem(5, 1, ToString1AfterComma(absorption.dark, "-"));
			list->SetItem(6, 1, ToString1AfterComma(resistance.bleed, "-"));
			list->SetItem(7, 1, ToString1AfterComma(resistance.poison, "-"));
			list->SetItem(8, 1, ToString1AfterComma(resistance.frost, "-"));
			list->SetItem(9, 1, ToString1AfterComma(resistance.curse, "-"));
		}
	};

	class Scaling final : public wxPanel
	{
		Value* str;
		Value* dex;
		Value* int_;
		Value* fth;
		Value* lck;

	public:
		Scaling(wxWindow* parent)
			: wxPanel(parent)
			, str(new Value(this, wxT("STR")))
			, dex(new Value(this, wxT("DEX")))
			, int_(new Value(this,wxT("INT")))
			, fth(new Value(this, wxT("FTH")))
			, lck(new Value(this, wxT("LCK")))
		{
			auto* sizer = new wxBoxSizer(wxVERTICAL);

			auto* sizerScaling = new wxBoxSizer(wxHORIZONTAL);
			sizerScaling->Add(str, 1, wxEXPAND);
			sizerScaling->Add(dex, 1, wxEXPAND);
			sizerScaling->Add(int_, 1, wxEXPAND);
			sizerScaling->Add(fth, 1, wxEXPAND);
			sizerScaling->Add(lck, 1, wxEXPAND);

			sizer->Add(new Preview::TextHeader(this, "Scaling", 12), 0, wxEXPAND | wxALL, 3);
			sizer->Add(sizerScaling, 0, wxEXPAND | wxALL, 3);

			this->SetSizer(sizer);
		}

		void UpdateWeapon(const invbuilder::Weapon& weapon, const int level, const invbuilder::Weapon::Infusion infusion)
		{
			using DB = invbuilder::Database;
			const auto& scaling = weapon.properties.at(infusion).level[level].scaling;

			str->SetLabelValue(DB::GetScalingGrade(scaling.strength));
			dex->SetLabelValue(DB::GetScalingGrade(scaling.dexterity));
			int_->SetLabelValue(DB::GetScalingGrade(scaling.intelligence));
			fth->SetLabelValue(DB::GetScalingGrade(scaling.faith));
			lck->SetLabelValue(DB::GetScalingGrade(scaling.luck));
		}
	};

	class Requirements final : public wxPanel
	{
		Value* str;
		Value* dex;
		Value* int_;
		Value* fth;

	public:
		Requirements(wxWindow* parent)
			: wxPanel(parent)
			, str(new Value(this, wxT("STR")))
			, dex(new Value(this, wxT("DEX")))
			, int_(new Value(this,wxT("INT")))
			, fth(new Value(this, wxT("FTH")))
		{
			auto* sizer = new wxBoxSizer(wxVERTICAL);

			auto* sizerReqs = new wxBoxSizer(wxHORIZONTAL);
			sizerReqs->Add(str, 1, wxEXPAND);
			sizerReqs->Add(dex, 1, wxEXPAND);
			sizerReqs->Add(int_, 1, wxEXPAND);
			sizerReqs->Add(fth, 1, wxEXPAND);

			sizer->Add(new Preview::TextHeader(this, "Requirements", 12), 0, wxEXPAND | wxALL, 3);
			sizer->Add(sizerReqs, 0, wxEXPAND | wxALL, 3);

			this->SetSizer(sizer);
		}

		void UpdateWeapon(const invbuilder::Weapon& weapon)
		{
			const auto& reqs = weapon.requirements;
			const auto& attribs = wxGetApp().GetSessionData().GetAttributes();

			str->SetLabelValue(ToStringInt(reqs.strength, "-"));
			dex->SetLabelValue(ToStringInt(reqs.dexterity, "-"));
			int_->SetLabelValue(ToStringInt(reqs.intelligence, "-"));
			fth->SetLabelValue(ToStringInt(reqs.faith, "-"));

			if (attribs.strength >= reqs.strength)
				str->SetColorWarning(0);
			else if (attribs.strength * 1.5 >= reqs.strength)
				str->SetColorWarning(1);
			else
				str->SetColorWarning(2);

			dex->SetColorWarning(attribs.dexterity >= reqs.dexterity ? 0 : 2);
			int_->SetColorWarning(attribs.intelligence >= reqs.intelligence ? 0 : 2);
			fth->SetColorWarning(attribs.faith >= reqs.faith ? 0 : 2);
		}
	};

	Properties* properties;
	AttackRating* attackRating;
	GuardAbsorption* guardAbsorption;
	Scaling* scaling;
	Requirements* requirements;

public:
	WeaponSimple(wxWindow* parent)
		: wxScrolledWindow(parent)
		, properties(new Properties(this))
		, attackRating(new AttackRating(this))
		, guardAbsorption(new GuardAbsorption(this))
		, scaling(new Scaling(this))
		, requirements(new Requirements(this))
	{
		auto* sizer = new wxBoxSizer(wxVERTICAL);

		auto* sizerRatings = new wxBoxSizer(wxHORIZONTAL);
		sizerRatings->Add(attackRating, 1, wxEXPAND | wxRIGHT, 3);
		sizerRatings->Add(guardAbsorption, 1, wxEXPAND);

		sizer->Add(properties, 0, wxEXPAND | wxALL, 5);
		sizer->Add(sizerRatings, 0, wxEXPAND | wxALL, 5);
		sizer->Add(scaling, 0, wxEXPAND | wxALL, 5);
		sizer->Add(requirements, 0, wxEXPAND | wxALL, 5);

		this->SetScrollRate(20, 20);
		this->SetSizer(sizer);
	}

	void UpdateSelection(const std::shared_ptr<WeaponContext> context)
	{
		const auto& weapon = wxGetApp().GetDatabase().GetWeapon(context->GetName());
		properties->UpdateWeapon(weapon);
		attackRating->UpdateWeapon(weapon, context->GetLevel(), context->GetInfusion());
		guardAbsorption->UpdateWeapon(weapon, context->GetLevel(), context->GetInfusion());
		scaling->UpdateWeapon(weapon, context->GetLevel(), context->GetInfusion());
		requirements->UpdateWeapon(weapon);
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
				wxGetApp().GetDatabase(), weapon.name.c_str(), infusion, level, attribs);
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
				ToStringInt(data.stability),
				ToString1AfterComma(data.absorption.physical),
				ToString1AfterComma(data.absorption.magic),
				ToString1AfterComma(data.absorption.fire),
				ToString1AfterComma(data.absorption.lightning),
				ToString1AfterComma(data.absorption.dark),
			});

			resistances->AddWeapon({name,
				ToString1AfterComma(data.resistance.bleed),
				ToString1AfterComma(data.resistance.poison),
				ToString1AfterComma(data.resistance.frost),
				ToString1AfterComma(data.resistance.curse),
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
				invbuilder::Database::ToString(weapon.type),
				ToString1AfterComma(weapon.weight),
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
				ToStringInt(weapon.requirements.strength),
				ToStringInt(weapon.requirements.dexterity),
				ToStringInt(weapon.requirements.intelligence),
				ToStringInt(weapon.requirements.faith)
			});
		}
	};

	/*class PageMoveset final : public wxWindow
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
	};*/

	PageOffensive* pageOffensive{nullptr};
	PageDefensive* pageDefensive{nullptr};
	PageProperties* pageProperties{nullptr};
	//PageMoveset* pageMoveset{nullptr};
	//PageRanges* pageRanges{nullptr};
	//PageCombos* pageCombos{nullptr};
	//PageRemarks* pageRemarks{nullptr};

public:
	WeaponBook(wxPanel* parent)
		: wxNotebook(parent, wxID_ANY)
		, pageOffensive(new PageOffensive{this})
		, pageDefensive(new PageDefensive{this})
		, pageProperties(new PageProperties{this})
		//, pageMoveset(new PageMoveset{this})
		//, pageRanges(new PageRanges{this})
		//, pageCombos(new PageCombos{this})
		//, pageRemarks(new PageRemarks{this})
	{
		this->AddPage(pageOffensive, "Offensive", true);
		this->AddPage(pageDefensive, "Defensive");
		this->AddPage(pageProperties, "Properties");
		//this->AddPage(pageMoveset, "Moveset");
		//this->AddPage(pageRanges, "Ranges");
		//this->AddPage(pageCombos, "Combos");
		//this->AddPage(pageRemarks, "Remarks");
	}

	void UpdateSelection(const WeaponContext::Vector& selection)
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
	, sizer(new wxBoxSizer{wxVERTICAL})
	, label(new WeaponLabel{GetContent()})
	, icon(new PreviewIcon{GetContent()})
	, book(new WeaponBook{GetContent()})
	, simple(new WeaponSimple{GetContent()})
{
	attributesListener->Register();
	selectionListener->Register();

	auto* top = new wxBoxSizer(wxHORIZONTAL);
	top->Add(label, 1, wxEXPAND);
	top->Add(icon);

	sizer->Add(top, 0, wxEXPAND | wxALL, 3);
	sizer->Add(simple, 1, wxEXPAND | wxALL, 3);
	sizer->Add(book, 1, wxEXPAND | wxALL, 3);
	sizer->Hide(book);

	SetShowSimple(true);
	GetContent()->SetSizer(sizer);
}

void Preview::SetShowSimple(const bool set)
{
	if (set && sizer->IsShown(book))
	{
		sizer->Hide(book);
		sizer->Show(simple);
		Layout();
	}
	else if (!set && sizer->IsShown(simple))
	{
		sizer->Hide(simple);
		sizer->Show(book);
		Layout();
	}
}

void Preview::OnSelectionUpdate()
{
	const auto& weakSelection = wxGetApp().GetSessionData().GetSelection();

	WeaponContext::Vector selection;
	selection.reserve(weakSelection.size());

	for (const auto& weaponContext : weakSelection)
		if (const auto ptr = weaponContext.lock(); ptr)
			selection.emplace_back(ptr);
		else
			assert(false && "weapon context should be valid at this point");

	if (!selection.size())
	{
		label->SetLabel("No weapon selected");
		label->SetNoWeapons();
		icon->ClearWeaponIcon();
	}
	else if (selection.size() == 1)
	{
		SetShowSimple(true);

		label->SetWeapon(selection[0]);
		icon->SetWeaponIcon(selection[0]->GetName());
		simple->UpdateSelection(selection[0]);
	}
	else
	{
		SetShowSimple(false);

		std::vector<std::string> weapons;
		for (const auto& weapon : selection)
			weapons.push_back(weapon->GetName());

		label->SetWeapons(selection);
		icon->SetWeaponIcons(std::move(weapons));
		book->UpdateSelection(selection);
	}
}
