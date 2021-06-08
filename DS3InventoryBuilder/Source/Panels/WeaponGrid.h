#pragma once

#include <wx/wx.h>
#include <Weapon.hpp>
#include <set>

class WeaponGrid final : public wxPanel
{
	using Sorting = invbuilder::Weapon::Sorting;
	using Infusion = invbuilder::Weapon::Infusion;
	using Type = invbuilder::Weapon::Type;

	using TypeSet = std::set<Type>;
	using InfusionSet = std::set<Infusion>;

	struct Range final
	{
		size_t start, end;
	};

	struct Card;
	using CardPtr = std::unique_ptr<Card>;
	std::vector<CardPtr> cards;
	std::vector<CardPtr> fallback;

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

	void InitializeAllWeapons();

	void AddSelectedWeapons(const int count);
	void RemoveSelectedWeapons();
	void DiscardSelection();

	bool MatchesFilters(const CardPtr&, const TypeSet&, const InfusionSet&) const;
	void SetFiltering(const TypeSet&, const InfusionSet&, const Sorting&);

	void Sort();
	void Sort(const Sorting&);

	void SetAllLevel(const int level, const Sorting&);
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
