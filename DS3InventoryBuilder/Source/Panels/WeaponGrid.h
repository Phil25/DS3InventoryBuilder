#pragma once

#include <wx/wx.h>

struct WeaponSorting final
{
	enum class Method
	{
		Default, Weight, AttackPower, GuardAbsorption, Effect
	};

	Method method;
	bool reverse;
};

class WeaponGrid final : public wxPanel
{
	struct Range final
	{
		size_t start, end;
	};

	struct Card;
	using CardPtr = std::unique_ptr<Card>;
	std::vector<CardPtr> cards;

	class SelectionManager;
	std::unique_ptr<SelectionManager> selection;

	int mouseOver{-1};
	int visibleRows{};
	int cardSize{};
	Range current{};
	WeaponSorting sorting{WeaponSorting::Method::Default, false};

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
	void SetSorting(const WeaponSorting& sorting);

private:
	void Sort();

	void Render(wxPaintEvent& e);
	void RenderItems(const bool fullRedraw=false);

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
