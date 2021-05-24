#include "WeaponGrid.h"

#include <AppMain.h>

#define RETURN_COMPARISON_ON_DIFFERENCE(a,b) \
	switch (Compare(a, b)) \
	{ \
	case -1: return true; \
	case 1: return false; \
	} \

class WeaponGrid::Card final : public wxPanel
{
	using Weapon = invbuilder::Weapon;
	using Infusion = Weapon::Infusion;

	const Weapon& data;
	Infusion infusion{Infusion::None};
	int level{10};

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

	auto GetInfusion() const
	{
		return infusion;
	}

	auto SetInfusion(const Infusion infusion)
	{
		this->infusion = infusion;
	}

	auto GetLevel() const
	{
		return level;
	}

	auto SetLevel(const int level)
	{
		this->level = level;
	}

private:
	void Render()
	{
		const auto& size = this->GetSize().x;
		auto dc = wxPaintDC{this};

		dc.DrawBitmap(wxGetApp().GetImage(data.name, size), 0, 0, false);
	}
};

namespace
{
	template <typename T>
	constexpr int Compare(const T& a, const T& b) noexcept
	{
		return (a < b) ? -1 : (a > b);
	}
}

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
		cards.emplace_back(new Card(this, name));

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
