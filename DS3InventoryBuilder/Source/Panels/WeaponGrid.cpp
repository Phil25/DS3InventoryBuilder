#include "WeaponGrid.h"

#include <AppMain.h>
#include <Menus/WeaponPopup.h>
#include <Calculator.h>
#include <algorithm>

#define RETURN_COMPARISON_ON_DIFFERENCE(a,b) \
	switch (Compare(a, b)) \
	{ \
	case -1: return true; \
	case 1: return false; \
	} \

namespace
{
	template <typename T>
	constexpr int Compare(const T& a, const T& b) noexcept
	{
		return (a < b) ? -1 : (a > b);
	}

	inline auto GetItemColor(const bool selected, const bool hovered)
	{
		switch (selected + hovered * 2)
		{
		// Hufflepuff palette: https://www.color-hex.com/color-palette/816
		case 0: return wxColor(114, 98, 85); // normal
		case 1: return wxColor(240, 199, 94); // selected
		case 2: return wxColor(55, 46, 41); // hovered
		case 3: return wxColor(236, 185, 57); // selected + hovered

		// blue tones
		//case 0: return wxColor(200, 200, 200); // normal
		//case 1: return wxColor(100, 255, 255); // selected
		//case 2: return wxColor(150, 240, 240); // hovered
		//case 3: return wxColor(100, 240, 240); // selected + hovered
		}

		assert(false && "invalid highlight combo");
		return wxColor{};
	}

	/*inline auto GetUniquenessColor(const bool unique)
	{
		// cool blue palette: https://www.color-hex.com/color-palette/30415
		return unique
			? wxColor(16, 125, 172)
			: wxColor(113, 199, 236);
	}*/

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
}

struct WeaponGrid::Card final
{
	using Weapon = invbuilder::Weapon;
	using Infusion = Weapon::Infusion;

	using WeaponContextPtr = std::shared_ptr<WeaponContext>;
	const WeaponContextPtr context;

	bool selected{false};
	bool hovered{false};
	int atPageFromSelection{0};
	int missingRequirements{0};

	wxPoint position{};

	Card(const int gridID, const int cardID, std::string name, const int level=10, const Infusion infusion=Infusion::None)
		: context(std::make_shared<WeaponContext>(gridID, cardID, std::move(name), level, infusion))
		, missingRequirements(CompareRequirements(wxGetApp().GetSessionData().GetAttributes(), wxGetApp().GetDatabase().GetWeapon(context->GetName()).requirements))
	{
	}

	Card(const WeaponContextPtr& context)
		: context(std::make_shared<WeaponContext>(*context))
		, missingRequirements(CompareRequirements(wxGetApp().GetSessionData().GetAttributes(), wxGetApp().GetDatabase().GetWeapon(context->GetName()).requirements))
	{
	}

	void Render(wxPaintDC& dc, const int size)
	{
		dc.SetBrush(GetItemColor(selected, hovered));
		dc.DrawRectangle(position, {size, size});

		dc.DrawBitmap(wxGetApp().GetImage(context->GetName(), size), position, false);

		switch (atPageFromSelection)
		{
		case -1: dc.DrawBitmap(wxGetApp().GetImage("Key_R2", size / 3), position.x + 2, position.y + 2, false); break;
		case 1: dc.DrawBitmap(wxGetApp().GetImage("Key_L2", size / 3), position.x + 2, position.y + 2, false); break;
		}

		if (context->GetInfusion() != Infusion::None)
		{
			const int infusionSize = size / 4;
			const int offset = size - infusionSize - 3;
			dc.DrawBitmap(wxGetApp().GetImage(invbuilder::Database::ToString(context->GetInfusion()), infusionSize), position.x + offset, position.y + offset, false);
		}

		switch (missingRequirements)
		{
		case 0: dc.DrawBitmap(wxGetApp().GetImage("NoStats", size / 4), position.x + size - size / 4 - 2, position.y + 2, false); break;
		case 1: dc.DrawBitmap(wxGetApp().GetImage("TwoHanded", size / 4), position.x + size - size / 4 - 2, position.y + 2, false); break;
		}

		/*{
			const int levelWidth = size / 5 + 5;
			dc.SetBrush(GetUniquenessColor(context->IsUnique()));
			dc.DrawRectangle({position.x + size - levelWidth, position.y}, {levelWidth, levelWidth / 2});
			dc.SetFont(wxFont{size / 10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD});
			dc.DrawText(std::to_string(context->GetLevel(true)), position.x + size - size / 5, position.y);
		}*/
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
			grid->cards[pageUpID]->atPageFromSelection = 1;

		if (const auto pageDownID = GetPageDownID(id, grid->cards.size()); pageDownID != id)
			grid->cards[pageDownID]->atPageFromSelection = -1;
	}

	void DeselectSingle(const int id)
	{
		grid->cards[id]->selected = false;
		grid->cards[GetPageUpID(id)]->atPageFromSelection = 0;
		grid->cards[GetPageDownID(id, grid->cards.size())]->atPageFromSelection = 0;
	}

	void Update()
	{
		SessionData::SelectionVector v;

		for (const int i : selection)
			v.push_back(grid->cards[i]->context);

		wxGetApp().GetSessionData().UpdateSelection(grid->gridID, std::move(v));
	}
};

class WeaponGrid::SortingManager final
{
public:
	SortingManager() = default;

	static auto GetComparator(const Sorting::Method method)
	{
		using M = Sorting::Method;

		switch (method)
		{
		case M::Default: return ComparatorDefault;
		case M::Weight: return ComparatorWeight;
		case M::AttackPower: return ComparatorAttackPower;
		case M::GuardAbsorption: return ComparatorDefault; // TODO
		case M::Effect: return ComparatorDefault; // TODO
		}

		assert(false && "invalid sorting method");
		return ComparatorDefault;
	}

	void Sort(std::vector<CardPtr>& cards)
	{
		const auto& sorting = wxGetApp().GetSessionData().GetSorting();

		if (sorting.reverse)
			std::sort(cards.rbegin(), cards.rend(), GetComparator(sorting.method));
		else
			std::sort(cards.begin(), cards.end(), GetComparator(sorting.method));
	}

private:
	static bool ComparatorDefault(const WeaponGrid::CardPtr& card1, const WeaponGrid::CardPtr& card2)
	{
		const auto& data1 = wxGetApp().GetDatabase().GetWeapon(card1->context->GetName());
		const auto& data2 = wxGetApp().GetDatabase().GetWeapon(card2->context->GetName());

		RETURN_COMPARISON_ON_DIFFERENCE(data1.orderID, data2.orderID);
		RETURN_COMPARISON_ON_DIFFERENCE(card1->context->GetInfusion(), card2->context->GetInfusion());
		RETURN_COMPARISON_ON_DIFFERENCE(card1->context->GetLevel(), card2->context->GetLevel());
		return true; // weapons are the same
	}

	static bool ComparatorWeight(const WeaponGrid::CardPtr& card1, const WeaponGrid::CardPtr& card2)
	{
		const auto& data1 = wxGetApp().GetDatabase().GetWeapon(card1->context->GetName());
		const auto& data2 = wxGetApp().GetDatabase().GetWeapon(card2->context->GetName());

		RETURN_COMPARISON_ON_DIFFERENCE(data1.weight, data2.weight);
		return ComparatorDefault(card1, card2);
	}

	static bool ComparatorAttackPower(const WeaponGrid::CardPtr& card1, const WeaponGrid::CardPtr& card2)
	{
		// TODO: this comparator is WRONG.
		// DS3 does really weird things when comparing weapons with very similar AR, and I can't figure out what
		// You can sometimes observe the order being ex. 62 > 61 > 62 even in-game, when checking out the value in the menu
		// However, if the difference is big enough (~2 AR), this works fine

		const auto attribs = wxGetApp().GetSessionData().GetAttributes();

		const auto& c1 = card1->context;
		const auto& c2 = card2->context;
		const auto& w1 = wxGetApp().GetDatabase().GetWeapon(c1->GetName());
		const auto& w2 = wxGetApp().GetDatabase().GetWeapon(c2->GetName());

		// DS3 always calculates two handed AR for bows/greatbows/crossbows
		const bool twoHanded = wxGetApp().GetSessionData().GetSorting().twoHanded;
		const bool twoHanded1 = twoHanded || invbuilder::Database::IsRanged(w1); 
		const bool twoHanded2 = twoHanded || invbuilder::Database::IsRanged(w2);

		const auto& [damages1, _1] = invbuilder::calculator::AttackRating(
			wxGetApp().GetDatabase(), c1->GetName().c_str(), c1->GetInfusion(), c1->GetLevel(), attribs, twoHanded1);

		const auto& [damages2, _2] = invbuilder::calculator::AttackRating(
			wxGetApp().GetDatabase(), c2->GetName().c_str(), c2->GetInfusion(), c2->GetLevel(), attribs, twoHanded2);

		const auto ar1 = static_cast<int>(damages1.Total());
		const auto ar2 = static_cast<int>(damages2.Total());

		RETURN_COMPARISON_ON_DIFFERENCE(ar1, ar2);
		return ComparatorDefault(card1, card2);
	}
};

WeaponGrid::WeaponGrid(wxWindow* parent, const bool fixed)
	: wxPanel(parent)
	, selection(std::make_unique<SelectionManager>(this))
	, sorting(std::make_unique<SortingManager>())
	, gridID(++GridID)
	, fixed(fixed)
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

void WeaponGrid::InitializeBaseWeapons()
{
	int cardID = 0;
	for (const auto& name : wxGetApp().GetDatabase().GetNames())
		cards.emplace_back(std::make_unique<Card>(gridID, cardID++, name));

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

void WeaponGrid::SetFiltering(/*filtering options*/)
{
	//assert(fallback && "cannot use filtering with uninitialized fallback vector");

	/*
	I.  Add optional<vector<CardPtr>> as a vector to keep getting cards from (CardPtr = shared)
	II. Pass a const bool to ctor to create it

	1. ClearSelection()
	2. cards.clean()
	3. Iterate fallback and copy CardPtr of revelant weapons
	5. Sort()

	But then cardIDs don't match... Gotta rethink this again later

	Consider using SetFiltering(some empty value) instead of InitializeBaseWeapons()
	*/
}

void WeaponGrid::Sort()
{
	const auto prev = selection->Get();
	std::vector<int> newSelection;
	selection->Clear(false);

	sorting->Sort(cards);

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

void WeaponGrid::UpdateRequirements()
{
	const auto& attr = wxGetApp().GetSessionData().GetAttributes();

	for (auto& card : cards)
	{
		const auto& reqs = wxGetApp().GetDatabase().GetWeapon(card->context->GetName()).requirements;
		card->missingRequirements = CompareRequirements(attr, reqs);
	}

	Refresh(false);
}

void WeaponGrid::OnRender(wxPaintEvent& e)
{
	auto dc = wxPaintDC{this};

	for (int i = std::max(current.start, 0ULL); i < std::min(current.end, cards.size()); ++i)
		cards[i]->Render(dc, cardSize);
}

void WeaponGrid::RenderCards(const bool fullRedraw)
{
	for (int pos = 0, i = std::max(current.start, 0ULL); i < std::min(current.end, cards.size()); ++i, ++pos)
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

	if (rotation < 0 && current.start < cards.size() - (static_cast<size_t>(visibleRows) * 5)) // down
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
	wxGetApp().GetSessionData().UpdateWeaponTransfer(gridID, 1);
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

	auto menu = WeaponPopup{gridID, fixed, selectedLevels, selectedInfusions};
	PopupMenu(&menu);

	Sort();
}

void WeaponGrid::UpdateMousePosition(const int x, const int y, const bool redraw)
{
	const auto id = GetIDFromPosition(x, y, current.start, cardSize);

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

int WeaponGrid::GridID = 0;
