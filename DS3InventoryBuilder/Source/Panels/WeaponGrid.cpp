#include "WeaponGrid.h"

#include <AppMain.h>

class WeaponGrid::Card final : public wxPanel
{
	const std::string name{"Dagger"};

public:
	Card(wxWindow* parent)
		: wxPanel(parent)
	{
		Bind(wxEVT_PAINT, [&](wxPaintEvent&) { this->Render(); });
	}

	void Render()
	{
		const auto& size = this->GetSize().x;
		auto dc = wxPaintDC{this};

		dc.DrawBitmap(wxGetApp().GetImage(name, size), 0, 0, false);
	}
};

WeaponGrid::WeaponGrid(wxWindow* parent)
	: wxPanel(parent)
{
	for (int i = 0; i < 286; ++i)
		cards.push_back(new Card(this));

	this->SetMinSize(wxSize(64 * 5, 128));
	this->SetMaxSize(wxSize(128 * 5, 99999));

	Bind(wxEVT_SIZE, &WeaponGrid::OnSize, this);
	Bind(wxEVT_MOUSEWHEEL, &WeaponGrid::OnMousewheel, this);
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
