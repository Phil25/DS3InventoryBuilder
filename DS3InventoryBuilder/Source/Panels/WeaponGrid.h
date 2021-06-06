#pragma once

#include <wx/wx.h>
#include <Weapon.hpp>

class WeaponGrid final : public wxPanel
{
	using Sorting = invbuilder::Weapon::Sorting;

	struct Range final
	{
		size_t start, end;
	};

	struct Card;
	using CardPtr = std::unique_ptr<Card>;
	std::vector<CardPtr> cards;

	class SelectionManager;
	std::unique_ptr<SelectionManager> selection;

	class SortingManager;
	std::unique_ptr<SortingManager> sorting;

	int mouseOver{-1};
	int visibleRows{};
	int cardSize{};
	Range current{};

	static int GridID;

public:
	const int gridID;
	const bool fixed;

	WeaponGrid(wxWindow* parent, const bool fixed=true);

	void InitializeBaseWeapons();

	void AddSelectedWeapons(const int count);
	void RemoveSelectedWeapons();
	void DiscardSelection();

	void SetFiltering();
	void Sort();
	void UpdateRequirements();

private:
	void OnRender(wxPaintEvent& e);
	void RenderCards(const bool fullRedraw=false);

	void OnSize(wxSizeEvent&);
	void OnMousewheel(wxMouseEvent&);

	void OnMouseMotion(wxMouseEvent&);
	void OnMouseLeave(wxMouseEvent&);

	void OnItemMouseLeft(wxMouseEvent&);
	void OnItemMouseDoubleLeft(wxMouseEvent&);
	void OnItemMouseRight(wxMouseEvent&);

	void UpdateMousePosition(const int x, const int y, const bool redraw=true);
	void OnItemEnterHover(const int id, const bool redraw);
	void OnItemLeaveHover(const int id, const bool redraw);

	friend bool ComparatorDefault(const WeaponGrid::CardPtr&, const WeaponGrid::CardPtr&);
	friend bool ComparatorWeight(const WeaponGrid::CardPtr&, const WeaponGrid::CardPtr&);
};
