#pragma once

#include <wx/wx.h>

class WeaponGrid final : public wxPanel
{
	struct Range final
	{
		size_t start, end;
	};

	class Card;
	std::vector<Card*> cards;

	int cardSize{};
	Range current{};

public:
	WeaponGrid(wxWindow* parent);

private:
	void OnSize(wxSizeEvent& e);
	void OnMousewheel(wxMouseEvent& e);
	
	void UpdateSize(const int width, const int height);
	void UpdateScroll(const Range& toHide, const Range& toShow, const Range& toUpdate);
};
