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

	inline auto GetPageUpID(const int id)
	{
		const auto rowsToTop = id / 5;
		return id - std::min(rowsToTop, 5) * 5;
	}

	inline auto GetPageDownID(const int id, const int size)
	{
		const auto max = size - 1;
		const auto rowsToBottom = (max - id) / 5;
		return id + std::min(rowsToBottom, 5) * 5;
	}
}

class WeaponGrid::Card final : public wxPanel
{
	using Weapon = invbuilder::Weapon;
	using Infusion = Weapon::Infusion;

	const Weapon& data;
	Infusion infusion{Infusion::None};
	int level{10};

	bool selected{false};
	bool hovered{false};
	int atPageFromSelection{};

public:
	Card(wxWindow* parent, const std::string& name)
		: wxPanel(parent)
		, data(wxGetApp().GetDatabase().GetWeapon(name))
	{
		Bind(wxEVT_PAINT, [&](wxPaintEvent&) { this->Render(); });
	}

	auto GetData() const -> const Weapon&
	{
		return data;
	}

	auto GetInfusion() const noexcept
	{
		return infusion;
	}

	auto SetInfusion(const Infusion infusion) noexcept
	{
		this->infusion = infusion;
	}

	auto GetLevel() const noexcept
	{
		return level;
	}

	auto SetLevel(const int level) noexcept
	{
		this->level = level;
	}

	bool IsSelected() const noexcept
	{
		return selected;
	}

	void SetSelected(const bool selected) noexcept
	{
		this->selected = selected;
		Refresh();
	}

	void SetHovered(const bool hovered) noexcept
	{
		this->hovered = hovered;
		Refresh();
	}

	void SetAtPageFromSelection(const int atPageFromSelection) noexcept
	{
		this->atPageFromSelection = atPageFromSelection;
		Refresh();
	}

private:
	void Render()
	{
		const auto& size = this->GetSize().x;
		auto dc = wxPaintDC{this};

		dc.SetBrush(GetItemColor(selected, hovered));
		dc.DrawRectangle({}, {size, size});

		dc.DrawBitmap(wxGetApp().GetImage(data.name, size), 0, 0, false);

		switch (atPageFromSelection)
		{
		case -1: dc.DrawBitmap(wxGetApp().GetImage("Key_R2", size / 3), 2, 2, false); break;
		case 1: dc.DrawBitmap(wxGetApp().GetImage("Key_L2", size / 3), 2, 2, false); break;
		}
	}
};

bool ComparatorDefault(const WeaponGrid::Card* card1, const WeaponGrid::Card* card2)
{
	RETURN_COMPARISON_ON_DIFFERENCE(card1->GetData().orderID, card2->GetData().orderID);
	RETURN_COMPARISON_ON_DIFFERENCE(card1->GetInfusion(), card2->GetInfusion());
	RETURN_COMPARISON_ON_DIFFERENCE(card1->GetLevel(), card2->GetLevel());
	return true; // weapons are the same
}

bool ComparatorWeight(const WeaponGrid::Card* card1, const WeaponGrid::Card* card2)
{
	RETURN_COMPARISON_ON_DIFFERENCE(card1->GetData().weight, card2->GetData().weight);
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
{
	this->SetMinSize(wxSize(64 * 5, 128));
	this->SetMaxSize(wxSize(128 * 5, 99999));

	Bind(wxEVT_SIZE, &WeaponGrid::OnSize, this);
	Bind(wxEVT_MOUSEWHEEL, &WeaponGrid::OnMousewheel, this);
}

void WeaponGrid::InitializeBaseWeapons()
{
	for (const auto& name : wxGetApp().GetDatabase().GetNames())
	{
		auto* card = new Card(this, name);
		card->Bind(wxEVT_LEFT_DOWN, &WeaponGrid::OnItemMouse, this);
		card->Bind(wxEVT_LEFT_UP, &WeaponGrid::OnItemMouse, this);
		card->Bind(wxEVT_ENTER_WINDOW, &WeaponGrid::OnItemEnterHover, this);
		card->Bind(wxEVT_LEAVE_WINDOW, &WeaponGrid::OnItemLeaveHover, this);
		cards.emplace_back(card);
	}

	Sort();
}

void WeaponGrid::SetSorting(const WeaponSorting& sorting)
{
	if (this->sorting.method != sorting.method &&
		this->sorting.reverse != sorting.reverse)
	{
		this->sorting = sorting;
		Sort();
	}
}

void WeaponGrid::AddWeapon(const Card* card)
{
}

void WeaponGrid::RemoveWeapon(const Card* card)
{
}

void WeaponGrid::Sort()
{
	if (sorting.reverse)
	{
		std::sort(cards.rbegin(), cards.rend(), GetComparatorFunction(sorting.method));
	}
	else
	{
		std::sort(cards.begin(), cards.end(), GetComparatorFunction(sorting.method));
	}
}

void WeaponGrid::OnSize(wxSizeEvent& e)
{
	UpdateSize(e.GetSize().x, e.GetSize().y);
}

void WeaponGrid::OnMousewheel(wxMouseEvent& e)
{
	const auto rotation = e.GetWheelRotation();

	if (rotation < 0 && current.start < cards.size() - 5) // down
	{
		UpdateScroll(
			{current.start, current.start + 5},
			{current.end, current.end + 5},
			{current.start + 5, current.end + 5});
	}
	else if (rotation > 0 && current.start > 0) // up
	{
		UpdateScroll(
			{current.end - 5, current.end},
			{current.start - 5, current.start},
			{current.start - 5, current.end - 5});
	}
}

void WeaponGrid::OnItemMouse(wxMouseEvent& e)
{
	int id = std::find(cards.begin(), cards.end(), e.GetEventObject()) - cards.begin();
	assert(0 <= id && id < cards.size() && "clicked on weapon card not found");

	if (e.LeftDown() && !selecting)
	{
		ClearSelection();
		SelectItemID(id);

		selection.start = id;
		selection.end = id;

		selecting = true;
	}
	else if (e.LeftUp() && selecting)
	{
		selecting = false;
	}
}

void WeaponGrid::OnItemEnterHover(wxMouseEvent& e)
{
	int id = std::find(cards.begin(), cards.end(), e.GetEventObject()) - cards.begin();
	assert(0 <= id && id < cards.size() && "clicked on weapon card not found");

	cards[id]->SetHovered(true);

	if (!selecting)
		return;

	ClearSelection();
	selection.end = id;

	for (int i = std::min(selection.start, selection.end); i <= std::max(selection.start, selection.end); ++i)
		SelectItemID(i);
}

void WeaponGrid::OnItemLeaveHover(wxMouseEvent& e)
{
	int id = std::find(cards.begin(), cards.end(), e.GetEventObject()) - cards.begin();
	assert(0 <= id && id < cards.size() && "clicked on weapon card not found");

	cards[id]->SetHovered(false);
}

void WeaponGrid::SelectItemID(const int id)
{
	cards[id]->SetSelected(true);
	cards[GetPageUpID(id)]->SetAtPageFromSelection(1);
	cards[GetPageDownID(id, cards.size())]->SetAtPageFromSelection(-1);
}

void WeaponGrid::DeselectItemID(const int id)
{
	cards[id]->SetSelected(false);
	cards[GetPageUpID(id)]->SetAtPageFromSelection(0);
	cards[GetPageDownID(id, cards.size())]->SetAtPageFromSelection(0);
}

void WeaponGrid::ClearSelection()
{
	for (int i = std::min(selection.start, selection.end); i <= std::max(selection.start, selection.end); ++i)
		DeselectItemID(i);
}

void WeaponGrid::UpdateSize(const int width, const int height)
{
	for (auto* card : cards)
		card->Hide();

	cardSize = width / 5;
	const auto visibleRows = height / cardSize + 1;
	current.end = current.start + static_cast<size_t>(visibleRows) * 5;

	for (int pos = 0, i = current.start; i < std::min(cards.size(), current.end); ++i, ++pos)
	{
		const int row = pos / 5;
		const int col = pos % 5;

		cards[i]->SetPosition(wxPoint(col * cardSize, row * cardSize));
		cards[i]->SetSize(cardSize, cardSize);
		cards[i]->Show();
	}
}

void WeaponGrid::UpdateScroll(const Range& toHide, const Range& toShow, const Range& toUpdate)
{
	for (int i = toHide.start; i < std::min(toHide.end, cards.size()); ++i)
		cards[i]->Hide();

	for (int i = toShow.start; i < std::min(toShow.end, cards.size()); ++i)
		cards[i]->Show();

	for (int pos = 0, i = toUpdate.start; i < std::min(toUpdate.end, cards.size()); ++i, ++pos)
	{
		const int row = pos / 5;
		const int col = pos % 5;

		cards[i]->SetSize(cardSize, cardSize);
		cards[i]->SetPosition(wxPoint(col * cardSize, row * cardSize));
	}

	current = toUpdate;
}
