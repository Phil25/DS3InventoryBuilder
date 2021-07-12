#include "WeaponGrid.h"

#include <AppMain.h>
#include <Menus/WeaponPopup.h>
#include <Utils/DrawWeapon.h>
#include <Calculator.h>
#include <Comparators.h>
#include <algorithm>

namespace
{
	inline auto GetItemColor(const bool selected, const bool hovered, const bool atPageFromSelection)
	{
		// Hufflepuff palette: https://www.color-hex.com/color-palette/816
		// Seafoam palette: https://www.color-hex.com/color-palette/1403

		if (selected)
			return hovered ? wxColor{236, 185, 57} : wxColor{240, 199, 94};

		switch (hovered + atPageFromSelection * 2)
		{
		case 0: return wxColor{114,98,85}; // normal
		case 1: return wxColor{55,46,41}; // hovered
		case 2: return wxColor{95,158,160}; // at page
		case 3: return wxColor{49,120,115}; // hovered + at page
		}

		assert(false && "invalid highlight combo");
		return wxColor{};
	}

	inline int GetIDFromPosition(const int x, const int y, const int startingCardID, const int cardSize) noexcept
	{
		const auto row = y / cardSize;
		const auto firstRowID = startingCardID + static_cast<size_t>(row) * 5;
		return firstRowID + x / cardSize;
	}

	inline auto GetPageUpID(const int id) noexcept
	{
		const auto rowsToTop = id / 5;
		return id - std::min(rowsToTop, 5) * 5;
	}

	inline auto GetPageDownID(const int id, const int size) noexcept
	{
		const auto max = size - 1;
		const auto rowsToBottom = (max - id) / 5;
		return id + std::min(rowsToBottom, 5) * 5;
	}

	inline auto CompareRequirements(const invbuilder::PlayerAttributes& attr, const invbuilder::PlayerAttributes& reqs) noexcept
	{
		if (attr.strength * 1.5 >= reqs.strength && attr.dexterity >= reqs.dexterity && attr.intelligence >= reqs.intelligence && attr.faith >= reqs.faith)
			return 1 + (attr.strength >= reqs.strength);
		else
			return 0;
	}

	inline auto GetRequirementsStatus(const std::string& name)
	{
		const auto& attribs = wxGetApp().GetSessionData().GetAttributes();
		const auto& reqs = wxGetApp().GetDatabase().GetWeapon(name).requirements;

		switch (CompareRequirements(attribs, reqs))
		{
		case 0: return WeaponContext::RequirementsStatus::NotMet;
		case 1: return WeaponContext::RequirementsStatus::TwoHanded;
		case 2: return WeaponContext::RequirementsStatus::Met;
		}

		assert(false && "invalid requirement status result");
		return WeaponContext::RequirementsStatus::NotMet;
	}

	inline bool MatchString(const std::string& filter, const std::string& name)
	{
		return name.end() != std::search(
			name.begin(), name.end(), filter.begin(), filter.end(),
			[](char c1, char c2) { return std::toupper(c1) == c2; }
		);
	}
}

struct WeaponGrid::Card final
{
	using Weapon = invbuilder::Weapon;
	using Infusion = Weapon::Infusion;
	using RequirementsStatus = WeaponContext::RequirementsStatus;

	using WeaponContextPtr = std::shared_ptr<WeaponContext>;
	const WeaponContextPtr context;

	bool selected{false};
	bool hovered{false};
	bool atPageFromSelection{false};

	wxPoint position{};

	Card(const int cardID, const std::string& name, const int level=10, const Infusion infusion=Infusion::None)
		: context(std::make_shared<WeaponContext>(cardID, name, level, infusion, GetRequirementsStatus(name)))
	{
	}

	Card(const WeaponContextPtr& context)
		: context(std::make_shared<WeaponContext>(*context))
	{
	}

	void Render(wxPaintDC& dc, const int size)
	{
		dc.SetBrush(GetItemColor(selected, hovered, atPageFromSelection));
		DrawWeapon(&dc, size, context->GetName(), context->GetInfusion(), position, static_cast<int>(context->GetRequirementsStatus()));
	}
};

class WeaponGrid::SelectionManager final
{
	bool selecting{false};
	Range current{};
	std::vector<int> selection;

	WeaponGrid* const grid;

public:
	SelectionManager(WeaponGrid* const grid) : grid(grid)
	{
	}

	auto IsSelecting() const
	{
		return selecting;
	}

	auto IsSelected(const int id) const
	{
		return std::find(selection.begin(), selection.end(), id) != selection.end();
	}

	const auto& Get() const
	{
		return selection;
	}

	void Set(decltype(selection) selection)
	{
		for (const int i : this->selection)
			DeselectSingle(i);

		this->selection = std::move(selection);

		for (const int i : this->selection)
			SelectSingle(i);

		Update();
	}

	void Begin(const int id)
	{
		current.start = id;
		current.end = id;

		Select(current);

		selecting = true;
	}

	void Extend(const int id)
	{
		assert(selecting && "not currently selecting");

		current.end = id;
		Select(current);
	}

	void End()
	{
		assert(selecting && "not currently selecting");

		selecting = false;
		Select(current);
	}

	void Select(const Range& range)
	{
		for (int i = std::min(range.start, range.end); i <= std::max(range.start, range.end); ++i)
		{
			SelectSingle(i);
			selection.push_back(i);
		}

		Update();
	}

	void Deselect(const Range& range)
	{
		for (int i = std::min(range.start, range.end); i <= std::max(range.start, range.end); ++i)
		{
			DeselectSingle(i);
			selection.erase(std::remove(selection.begin(), selection.end(), i), selection.end());
		}

		Update();
	}

	void Clear(const bool triggerUpdate=true)
	{
		for (const int i : selection)
			DeselectSingle(i);

		selection.clear();

		if (triggerUpdate)
			Update();
	}

private:
	void SelectSingle(const int id)
	{
		grid->cards[id]->selected = true;

		if (const auto pageUpID = GetPageUpID(id); pageUpID != id)
			grid->cards[pageUpID]->atPageFromSelection = true;

		if (const auto pageDownID = GetPageDownID(id, grid->cards.size()); pageDownID != id)
			grid->cards[pageDownID]->atPageFromSelection = true;
	}

	void DeselectSingle(const int id)
	{
		grid->cards[id]->selected = false;
		grid->cards[GetPageUpID(id)]->atPageFromSelection = 0;
		grid->cards[GetPageDownID(id, grid->cards.size())]->atPageFromSelection = false;
	}

	void Update()
	{
		WeaponContext::WeakVector v;

		for (const int i : selection)
			v.push_back(grid->cards[i]->context);

		wxGetApp().GetSessionData().UpdateSelection(grid->role, std::move(v));
	}
};

class WeaponGrid::SortingManager final
{
	using Method = Sorting::Method;

public:
	SortingManager() = default;

	static auto GetComparator(const Method method)
	{
		using M = Method;

		switch (method)
		{
		case M::Default: return Comparator<M::Default>;
		case M::Weight: return Comparator<M::Weight>;
		case M::AttackPower: return Comparator<M::AttackPower>;
		case M::GuardAbsorption: return Comparator<M::GuardAbsorption>;
		case M::Effect: return Comparator<M::Effect>;
		case M::AttackPowerPrecise: return Comparator<M::AttackPowerPrecise>;
		case M::AttackPowerPreciseTwoHanded: return Comparator<M::AttackPowerPreciseTwoHanded>;
		case M::AttackPowerPreciseTwoHandedIfRequired: return Comparator<M::AttackPowerPreciseTwoHandedIfRequired>;
		case M::Stability: return Comparator<M::Stability>;
		case M::StabilityThenGuardAbsorption: return Comparator<M::StabilityThenGuardAbsorption>;
		}

		assert(false && "invalid sorting method");
		return Comparator<M::Default>;
	}

	void Sort(std::vector<CardPtr>& cards, const Sorting& sorting)
	{
		if (sorting.reverse)
			std::sort(cards.rbegin(), cards.rend(), GetComparator(sorting.method));
		else
			std::sort(cards.begin(), cards.end(), GetComparator(sorting.method));
	}

private:
	template <Method M>
	static bool Comparator(const WeaponGrid::CardPtr& card1, const WeaponGrid::CardPtr& card2)
	{
		namespace comps = invbuilder::comparators;

		const auto& weap1 = wxGetApp().GetDatabase().GetWeapon(card1->context->GetName());
		const auto& inf1 = card1->context->GetInfusion();
		const auto& lvl1 = card1->context->GetLevel();

		const auto& weap2 = wxGetApp().GetDatabase().GetWeapon(card2->context->GetName());
		const auto& inf2 = card2->context->GetInfusion();
		const auto& lvl2 = card2->context->GetLevel();

		if constexpr (M == Method::Default)
		{
			return comps::Default(weap1, inf1, lvl1, weap2, inf2, lvl2);
		}
		else if constexpr (M == Method::Weight)
		{
			return comps::Weight(weap1, inf1, lvl1, weap2, inf2, lvl2);
		}
		else if constexpr (M == Method::AttackPower)
		{
			const auto& db = wxGetApp().GetDatabase();
			const auto& attribs = wxGetApp().GetSessionData().GetAttributes();
			const bool twoHanded = wxGetApp().GetSessionData().GetSorting().twoHanded;
			return comps::AttackPower(weap1, inf1, lvl1, weap2, inf2, lvl2, db, attribs, twoHanded);
		}
		else if constexpr (M == Method::GuardAbsorption)
		{
			return comps::GuardAbsorption(weap1, inf1, lvl1, weap2, inf2, lvl2);
		}
		else if constexpr (M == Method::Effect)
		{
			return comps::Effect(weap1, inf1, lvl1, weap2, inf2, lvl2);
		}
		else if constexpr (M == Method::AttackPowerPrecise)
		{
			const auto& db = wxGetApp().GetDatabase();
			const auto& attribs = wxGetApp().GetSessionData().GetAttributes();
			return comps::AttackPowerPrecise(weap1, inf1, lvl1, weap2, inf2, lvl2, db, attribs);
		}
		else if constexpr (M == Method::AttackPowerPreciseTwoHanded)
		{
			const auto& db = wxGetApp().GetDatabase();
			const auto& attribs = wxGetApp().GetSessionData().GetAttributes();
			return comps::AttackPowerPreciseTwoHanded(weap1, inf1, lvl1, weap2, inf2, lvl2, db, attribs);
		}
		else if constexpr (M == Method::AttackPowerPreciseTwoHandedIfRequired)
		{
			const auto& db = wxGetApp().GetDatabase();
			const auto& attribs = wxGetApp().GetSessionData().GetAttributes();
			return comps::AttackPowerPreciseTwoHandedIfRequired(weap1, inf1, lvl1, weap2, inf2, lvl2, db, attribs);
		}
		else if constexpr (M == Method::Stability)
		{
			return comps::Stability(weap1, inf1, lvl1, weap2, inf2, lvl2);
		}
		else if constexpr (M == Method::StabilityThenGuardAbsorption)
		{
			return comps::StabilityThenGuardAbsorption(weap1, inf1, lvl1, weap2, inf2, lvl2);
		}

		return true;
	}
};

WeaponGrid::WeaponGrid(wxWindow* parent, const GridRole role)
	: wxPanel(parent)
	, selection(std::make_unique<SelectionManager>(this))
	, sorting(std::make_unique<SortingManager>())
	, role(role)
{
	this->SetMinSize(wxSize(64 * 5, 128));
	this->SetMaxSize(wxSize(128 * 5, 99999));

	this->Bind(wxEVT_SIZE, &WeaponGrid::OnSize, this);

	this->Bind(wxEVT_MOUSEWHEEL, &WeaponGrid::OnMousewheel, this);
	this->Bind(wxEVT_MOTION, &WeaponGrid::OnMouseMotion, this);
	this->Bind(wxEVT_LEAVE_WINDOW, &WeaponGrid::OnMouseLeave, this);

	this->Bind(wxEVT_LEFT_DOWN, &WeaponGrid::OnItemMouseLeft, this);
	this->Bind(wxEVT_LEFT_UP, &WeaponGrid::OnItemMouseLeft, this);
	this->Bind(wxEVT_LEFT_DCLICK, &WeaponGrid::OnItemMouseDoubleLeft, this);
	this->Bind(wxEVT_RIGHT_DOWN, &WeaponGrid::OnItemMouseRight, this);

	this->Bind(wxEVT_PAINT, &WeaponGrid::OnRender, this);
}

void WeaponGrid::InitializeAllWeapons()
{
	using Infusion = invbuilder::Weapon::Infusion;

	int cardID = 0;
	for (const auto& name : wxGetApp().GetDatabase().GetNames())
	{
		fallback.emplace_back(std::make_unique<Card>(cardID++, name));

		if (wxGetApp().GetDatabase().GetWeapon(name).infusable)
			for (int infusion = 1; infusion < static_cast<int>(Infusion::Size); ++infusion)
				fallback.emplace_back(std::make_unique<Card>(cardID++, name, 10, static_cast<Infusion>(infusion)));
	}

	Sort();
}

void WeaponGrid::AddSelectedWeapons(const int count)
{
	const auto& weaponSelection = wxGetApp().GetSessionData().GetSelection();

	for (const auto& weakPtr : weaponSelection)
		if (const auto ptr = weakPtr.lock(); ptr)
			for (int i = 0; i < count; ++i)
				cards.emplace_back(std::make_unique<Card>(ptr));

	Sort();
}

void WeaponGrid::RemoveSelectedWeapons()
{
	const auto& weaponSelection = wxGetApp().GetSessionData().GetSelection();

	std::vector<int> toDelete;
	toDelete.reserve(weaponSelection.size());

	for (const auto& weakPtr : weaponSelection)
		if (const auto ptr = weakPtr.lock(); ptr)
			toDelete.push_back(ptr->GetCardID());

	selection->Clear();
	std::sort(toDelete.begin(), toDelete.end());

	int counter = 0;
	for (const int index : toDelete)
	{
		cards.erase(cards.begin() + index + counter);
		--counter;
	}

	Sort();
}

void WeaponGrid::DiscardSelection()
{
	if (!selection->Get().empty())
	{
		selection->Clear(false);
		Refresh(false);
	}
}

bool WeaponGrid::MatchesFilters(const std::string& filter, const CardPtr& card, const TypeSet& types, const InfusionSet& infusions) const
{
	const auto& name = card->context->GetName();
	const auto& weapon = wxGetApp().GetDatabase().GetWeapon(name); // TODO: cache type
	return types.count(weapon.type) && infusions.count(card->context->GetInfusion()) && (filter.empty() || MatchString(filter, name));
}

void WeaponGrid::SetFiltering(std::string filter, const TypeSet& types, const InfusionSet& infusions, const Sorting& sortingOverride)
{
	selection->Clear();
	UpdateMousePosition(-1, true);

	std::move(cards.begin(), cards.end(), std::back_inserter(fallback));
	cards.clear();

	std::transform(filter.begin(), filter.end(), filter.begin(), ::toupper);

	for (auto it = fallback.begin(); it != fallback.end();)
	{
		auto& card = *it;

		if (MatchesFilters(filter, card, types, infusions))
		{
			cards.emplace_back(std::move(card));
			it = fallback.erase(it);
		}
		else ++it;
	}

	Sort(sortingOverride);
}

void WeaponGrid::Sort()
{
	// only inventory grid role should use session sorting
	if (role == GridRole::Inventory)
		Sort(wxGetApp().GetSessionData().GetSorting());
}

void WeaponGrid::Sort(const Sorting& desiredSorting)
{
	const auto prev = selection->Get();
	std::vector<int> newSelection;
	selection->Clear(false);

	sorting->Sort(cards, desiredSorting);

	int cardID = 0;
	for (auto& card : cards)
	{
		if (std::find(prev.begin(), prev.end(), card->context->GetCardID()) != prev.end())
			newSelection.push_back(cardID);

		card->context->SetCardID(cardID, card->context.use_count());
		++cardID;
	}

	selection->Set(std::move(newSelection));
	RenderCards();
}

void WeaponGrid::SetAllLevel(const int level, const Sorting& sortingOverride)
{
	selection->Clear();

	for (auto& card : cards)
		card->context->SetLevel(level);

	for (auto& card : fallback)
		card->context->SetLevel(level);

	Sort(sortingOverride);
}

void WeaponGrid::UpdateRequirements()
{
	const auto& attr = wxGetApp().GetSessionData().GetAttributes();

	for (auto& card : cards)
		card->context->SetRequirementsStatus(GetRequirementsStatus(card->context->GetName()));

	for (auto& card : fallback)
		card->context->SetRequirementsStatus(GetRequirementsStatus(card->context->GetName()));

	Refresh(false);
}

auto WeaponGrid::Retrieve() const -> WeaponContext::WeakVector
{
	WeaponContext::WeakVector v;

	for (const auto& card : cards)
		v.push_back(card->context);

	return v;
}

void WeaponGrid::Override(const WeaponContext::Vector& weapons)
{
	selection->Clear();
	cards.clear();

	for (const auto& ptr : weapons)
		cards.emplace_back(std::make_unique<Card>(ptr));

	Sort();
	UpdateRequirements();
}

void WeaponGrid::OnRender(wxPaintEvent& e)
{
	auto dc = wxPaintDC{this};

	for (int i = std::max(current.start, static_cast<size_t>(0)); i < std::min(current.end, cards.size()); ++i)
		cards[i]->Render(dc, cardSize);

	dc.SetPen(scrollGuide);
	const int width = scrollGuide.GetWidth() / 2;

	if (current.start > 0)
		dc.DrawLine(0, width, cardSize * 5, width);

	if (current.end < cards.size())
		dc.DrawLine(0, cardSize * visibleRows - width, cardSize * 5, cardSize * visibleRows - width);
}

void WeaponGrid::RenderCards(const bool fullRedraw)
{
	if (current.start >= cards.size())
	{
		current.start = 0;
		current.end = static_cast<size_t>(visibleRows) * 5;
	}

	for (int pos = 0, i = std::max(current.start, static_cast<size_t>(0)); i < std::min(current.end, cards.size()); ++i, ++pos)
	{
		const int row = pos / 5;
		const int col = pos % 5;
		cards[i]->position = {col * cardSize, row * cardSize};
	}

	// redraw needed always if background is visible at the end
	Refresh(fullRedraw || current.end > cards.size());
}

void WeaponGrid::OnSize(wxSizeEvent& e)
{
	cardSize = e.GetSize().x / 5;
	visibleRows = e.GetSize().y / cardSize;
	current.end = current.start + static_cast<size_t>(visibleRows) * 5;
	RenderCards(true);
}

void WeaponGrid::OnMousewheel(wxMouseEvent& e)
{
	const auto rotation = e.GetWheelRotation();

	if (rotation < 0 && current.end < cards.size()) // down
	{
		current.start += 5;
		current.end += 5;
		UpdateMousePosition(e.GetPosition().x, e.GetPosition().y, false);
		RenderCards();
	}
	else if (rotation > 0 && current.start > 0) // up
	{
		current.start -= 5;
		current.end -= 5;
		UpdateMousePosition(e.GetPosition().x, e.GetPosition().y, false);
		RenderCards();
	}
}

void WeaponGrid::OnMouseMotion(wxMouseEvent& e)
{
	UpdateMousePosition(e.GetPosition().x, e.GetPosition().y);
}

void WeaponGrid::OnMouseLeave(wxMouseEvent&)
{
	OnItemLeaveHover(mouseOver, true);
	mouseOver = -1;

	if (selection->IsSelecting())
	{
		selection->Clear(false);
		selection->End();
	}
}

void WeaponGrid::OnItemMouseLeft(wxMouseEvent& e)
{
	if (mouseOver < 0 || mouseOver >= cards.size())
		return;

	if (e.LeftDown() && !selection->IsSelecting())
	{
		selection->Clear();
		selection->Begin(mouseOver);
	}
	else if (e.LeftUp() && selection->IsSelecting())
	{
		selection->Clear(false);
		selection->End();
	}

	Refresh(false);
}

void WeaponGrid::OnItemMouseDoubleLeft(wxMouseEvent&)
{
	wxGetApp().GetSessionData().UpdateWeaponTransfer(OtherGrid(role), 1);
}

void WeaponGrid::OnItemMouseRight(wxMouseEvent& e)
{
	if (mouseOver < 0 || mouseOver >= cards.size())
		return;

	if (!selection->IsSelected(mouseOver))
	{
		selection->Clear();
		selection->Select({mouseOver * 1ULL, mouseOver * 1ULL});
		Refresh(false);
	}

	int selectedLevels{0};
	int selectedInfusions{0};
	for (const auto i : selection->Get())
	{
		selectedLevels |= 1 << cards[i]->context->GetLevel();
		selectedInfusions |= 1 << static_cast<int>(cards[i]->context->GetInfusion());
	}

	auto menu = WeaponPopup{role, selectedLevels, selectedInfusions};
	PopupMenu(&menu);

	if (menu.ShouldSelectAll())
	{
		selection->Clear();
		selection->Select({0ULL, cards.size() - 1});
		Refresh(false);
	}
	else if (menu.WereWeaponsTransferred() || menu.WereWeaponsAltered())
	{
		Sort();
	}
}

void WeaponGrid::UpdateMousePosition(const int x, const int y, const bool redraw)
{
	const auto id = GetIDFromPosition(x, y, current.start, cardSize);
	UpdateMousePosition(id, redraw);
}

void WeaponGrid::UpdateMousePosition(const int id, const bool redraw)
{
	if (id == mouseOver)
		return;

	OnItemLeaveHover(mouseOver, redraw);
	mouseOver = id;
	OnItemEnterHover(mouseOver, redraw);
}

void WeaponGrid::OnItemEnterHover(const int id, const bool redraw)
{
	if (id < 0 || id >= cards.size())
		return;

	cards[id]->hovered = true;
	bool wasRefreshed = false;

	if (selection->IsSelecting())
	{
		selection->Clear();
		selection->Extend(id);

		Refresh(false);
		wasRefreshed = true;
	}

	if (redraw && !wasRefreshed)
		RefreshRect({cards[id]->position.x, cards[id]->position.y, cardSize, cardSize}, false);
}

void WeaponGrid::OnItemLeaveHover(const int id, const bool redraw)
{
	if (id < 0 || id >= cards.size())
		return;

	cards[id]->hovered = false;

	if (redraw)
		RefreshRect({cards[id]->position.x, cards[id]->position.y, cardSize, cardSize}, false);
}
