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

	int mouseOver{-1};
	int visibleRows{};
	int cardSize{};
	Range current{};
	Range selection{};

	WeaponSorting sorting{WeaponSorting::Method::Default, false};

	bool selecting{false};

public:
	WeaponGrid(wxWindow* parent);

	void InitializeBaseWeapons();

	void SetSorting(const WeaponSorting& sorting);

private:
	void AddWeapon(const Card* card);
	void RemoveWeapon(const Card* card);

	void Sort();

	void Render(wxPaintEvent& e);
	void RenderItems(const bool fullRedraw=false);


	void OnSize(wxSizeEvent& e);
	void OnMousewheel(wxMouseEvent& e);

	void OnMouseMotion(wxMouseEvent& e);
	void OnMouseLeave(wxMouseEvent& e);
	void OnItemMouse(wxMouseEvent& e);

	void UpdateMousePosition(const int x, const int y, const bool redraw=true);
	void OnItemEnterHover(const int id, const bool redraw);
	void OnItemLeaveHover(const int id, const bool redraw);
	
	void SelectItemID(const int id);
	void DeselectItemID(const int id);
	void ClearSelection();

	friend bool ComparatorDefault(const WeaponGrid::CardPtr&, const WeaponGrid::CardPtr&);
	friend bool ComparatorWeight(const WeaponGrid::CardPtr&, const WeaponGrid::CardPtr&);
};
