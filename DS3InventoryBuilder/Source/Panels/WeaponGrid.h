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

	class Card;
	std::vector<Card*> cards;

	int cardSize{};
	Range current{};

	WeaponSorting sorting{WeaponSorting::Method::Default, false};

public:
	WeaponGrid(wxWindow* parent);

	void InitializeBaseWeapons();

	void SetSorting(const WeaponSorting& sorting);

private:
	void AddWeapon(const Card* card);
	void RemoveWeapon(const Card* card);

	void Sort();

	void OnSize(wxSizeEvent& e);
	void OnMousewheel(wxMouseEvent& e);
	
	void UpdateSize(const int width, const int height);
	void UpdateScroll(const Range& toHide, const Range& toShow, const Range& toUpdate);

	friend bool ComparatorDefault(const WeaponGrid::Card*, const WeaponGrid::Card*);
	friend bool ComparatorWeight(const WeaponGrid::Card*, const WeaponGrid::Card*);
};
