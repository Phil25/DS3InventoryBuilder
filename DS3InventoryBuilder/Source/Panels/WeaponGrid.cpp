#include "WeaponGrid.h"

#include <AppMain.h>
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
		// Hufflepuff palette -- https://www.color-hex.com/color-palette/816
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
		return wxColor();
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
}

struct WeaponGrid::Card final
{
	using Weapon = invbuilder::Weapon;
	using Infusion = Weapon::Infusion;

	std::shared_ptr<WeaponContext> context;

	bool selected{false};
	bool hovered{false};
	int atPageFromSelection{0};

	wxPoint position{};

	Card(const int gridID, const int cardID, std::string name, const int level=10, const Infusion infusion=Infusion::None)
		: context(std::make_shared<WeaponContext>(gridID, cardID, std::move(name), level, infusion))
	{
	}

	void UpdateContext(const int gridID, const int cardID)
	{
		// UpdateContext() should be done only after sorting, and before that selection should be emptied, removing the preview
		assert(context.use_count() == 1 && "only this card should own the weapon context at this time");
		context = std::make_shared<WeaponContext>(gridID, cardID, context->name, context->GetLevel(), context->GetInfusion());
	}

	void Render(wxPaintDC& dc, const int size)
	{
		dc.SetBrush(GetItemColor(selected, hovered));
		dc.DrawRectangle(position, {size, size});

		dc.DrawBitmap(wxGetApp().GetImage(context->name, size), position, false);

		switch (atPageFromSelection)
		{
		case -1: dc.DrawBitmap(wxGetApp().GetImage("Key_R2", size / 3), position.x + 2, position.y + 2, false); break;
		case 1: dc.DrawBitmap(wxGetApp().GetImage("Key_L2", size / 3), position.x + 2, position.y + 2, false); break;
		}
	}
};

bool ComparatorDefault(const WeaponGrid::CardPtr& card1, const WeaponGrid::CardPtr& card2)
{
	const auto& data1 = wxGetApp().GetDatabase().GetWeapon(card1->context->name);
	const auto& data2 = wxGetApp().GetDatabase().GetWeapon(card2->context->name);

	RETURN_COMPARISON_ON_DIFFERENCE(data1.orderID, data2.orderID);
	RETURN_COMPARISON_ON_DIFFERENCE(card1->context->GetInfusion(), card2->context->GetInfusion());
	RETURN_COMPARISON_ON_DIFFERENCE(card1->context->GetLevel(), card2->context->GetLevel());
	return true; // weapons are the same
}

bool ComparatorWeight(const WeaponGrid::CardPtr& card1, const WeaponGrid::CardPtr& card2)
{
	const auto& data1 = wxGetApp().GetDatabase().GetWeapon(card1->context->name);
	const auto& data2 = wxGetApp().GetDatabase().GetWeapon(card2->context->name);

	RETURN_COMPARISON_ON_DIFFERENCE(data1.weight, data2.weight);
	return ComparatorDefault(card1, card2);
}

namespace
{
	inline auto* GetComparatorFunction(const WeaponSorting::Method method)
	{
		using M = WeaponSorting::Method;
		switch (method)
		{
		case M::Default: return ComparatorDefault;
		case M::Weight: return ComparatorWeight;
		case M::AttackPower: return ComparatorDefault; // TODO
		case M::GuardAbsorption: return ComparatorDefault; // TODO
		case M::Effect: return ComparatorDefault; // TODO
		}

		assert(false && "invalid sorting method");
		return ComparatorDefault;
	}
}

WeaponGrid::WeaponGrid(wxWindow* parent)
	: wxPanel(parent)
	, gridID(++GridID)
{
	this->SetMinSize(wxSize(64 * 5, 128));
	this->SetMaxSize(wxSize(128 * 5, 99999));

	this->Bind(wxEVT_SIZE, &WeaponGrid::OnSize, this);

	this->Bind(wxEVT_MOUSEWHEEL, &WeaponGrid::OnMousewheel, this);
	this->Bind(wxEVT_MOTION, &WeaponGrid::OnMouseMotion, this);
	this->Bind(wxEVT_LEAVE_WINDOW, &WeaponGrid::OnMouseLeave, this);
	this->Bind(wxEVT_LEFT_DOWN, &WeaponGrid::OnItemMouse, this);
	this->Bind(wxEVT_LEFT_UP, &WeaponGrid::OnItemMouse, this);
	this->Bind(wxEVT_LEFT_DCLICK, &WeaponGrid::OnItemMouseDoubleClick, this);

	this->Bind(wxEVT_PAINT, &WeaponGrid::Render, this);
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
	{
		const auto ptr = weakPtr.lock();
		for (int i = 0; i < count; ++i)
			cards.emplace_back(std::make_unique<Card>(gridID, 0, ptr->name, ptr->GetLevel(), ptr->GetInfusion()));
	}

	Sort();
}

void WeaponGrid::RemoveSelectedWeapons()
{
	const auto& weaponSelection = wxGetApp().GetSessionData().GetSelection();

	std::vector<int> toDelete;
	toDelete.reserve(weaponSelection.size());

	for (const auto& weakPtr : weaponSelection)
		if (const auto ptr = weakPtr.lock(); ptr)
			toDelete.push_back(ptr->cardID);

	ClearSelection();
	std::sort(toDelete.begin(), toDelete.end());

	int counter = 0;
	for (const int index : toDelete)
	{
		cards.erase(cards.begin() + index + counter);
		--counter;
	}

	Sort();
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

void WeaponGrid::SetSorting(const WeaponSorting& sorting)
{
	if (this->sorting.method == sorting.method && this->sorting.reverse == sorting.reverse)
		return;

	this->sorting = sorting;
	Sort();
}

void WeaponGrid::Sort()
{
	ClearSelection();

	if (sorting.reverse)
		std::sort(cards.rbegin(), cards.rend(), GetComparatorFunction(sorting.method));
	else
		std::sort(cards.begin(), cards.end(), GetComparatorFunction(sorting.method));

	int cardID = 0;
	for (auto& card : cards)
		card->UpdateContext(gridID, cardID++);

	RenderItems();
}

void WeaponGrid::Render(wxPaintEvent& e)
{
	auto dc = wxPaintDC{this};

	for (int i = std::max(current.start, 0ULL); i < std::min(current.end, cards.size()); ++i)
		cards[i]->Render(dc, cardSize);
}

void WeaponGrid::RenderItems(const bool fullRedraw)
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
	RenderItems(true);
}

void WeaponGrid::OnMousewheel(wxMouseEvent& e)
{
	const auto rotation = e.GetWheelRotation();

	if (rotation < 0 && current.start < cards.size() - (static_cast<size_t>(visibleRows) * 5)) // down
	{
		current.start += 5;
		current.end += 5;
		UpdateMousePosition(e.GetPosition().x, e.GetPosition().y, false);
		RenderItems();
	}
	else if (rotation > 0 && current.start > 0) // up
	{
		current.start -= 5;
		current.end -= 5;
		UpdateMousePosition(e.GetPosition().x, e.GetPosition().y, false);
		RenderItems();
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
}

void WeaponGrid::OnItemMouse(wxMouseEvent& e)
{
	if (mouseOver < 0 || mouseOver >= cards.size())
		return;

	if (e.LeftDown() && !selecting)
	{
		ClearSelection();

		selection.start = mouseOver;
		selection.end = mouseOver;

		SelectItemID(mouseOver);

		selectedIDs.push_back(mouseOver);
		UpdateSelection();

		selecting = true;
	}
	else if (e.LeftUp() && selecting)
	{
		selecting = false;

		selectedIDs.clear();
		for (int i = std::min(selection.start, selection.end); i <= std::max(selection.start, selection.end); ++i)
			selectedIDs.push_back(i);

		UpdateSelection();
	}

	Refresh();
}

void WeaponGrid::OnItemMouseDoubleClick(wxMouseEvent&)
{
	wxGetApp().GetSessionData().UpdateWeaponTransfer(gridID, 1);
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
	if (id < 0 || id >= cards.size()) return;

	cards[id]->hovered = true;
	bool wasRefreshed = false;

	if (selecting)
	{
		ClearSelection();
		selection.end = id;

		for (int i = std::min(selection.start, selection.end); i <= std::max(selection.start, selection.end); ++i)
		{
			SelectItemID(i);
			selectedIDs.push_back(i);
		}

		UpdateSelection();
		Refresh();
		wasRefreshed = true;
	}

	if (redraw && !wasRefreshed)
		RefreshRect({cards[id]->position.x, cards[id]->position.y, cardSize, cardSize});
}

void WeaponGrid::OnItemLeaveHover(const int id, const bool redraw)
{
	if (id < 0 || id >= cards.size()) return;

	cards[id]->hovered = false;

	if (redraw)
		RefreshRect({cards[id]->position.x, cards[id]->position.y, cardSize, cardSize});
}

void WeaponGrid::SelectItemID(const int id)
{
	cards[id]->selected = true;

	if (const auto pageUpID = GetPageUpID(id); pageUpID != id)
		cards[pageUpID]->atPageFromSelection = 1;

	if (const auto pageDownID = GetPageDownID(id, cards.size()); pageDownID != id)
		cards[pageDownID]->atPageFromSelection = -1;
}

void WeaponGrid::DeselectItemID(const int id)
{
	cards[id]->selected = false;
	cards[GetPageUpID(id)]->atPageFromSelection = 0;
	cards[GetPageDownID(id, cards.size())]->atPageFromSelection = 0;
}

void WeaponGrid::ClearSelection()
{
	for (const int i : selectedIDs)
		DeselectItemID(i);

	selectedIDs.clear();
	UpdateSelection();
}

void WeaponGrid::UpdateSelection()
{
	SessionData::SelectionVector v;

	for (const int i : selectedIDs)
		v.push_back(cards[i]->context);

	wxGetApp().GetSessionData().UpdateSelection(std::move(v));
}

int WeaponGrid::GridID = 0;
