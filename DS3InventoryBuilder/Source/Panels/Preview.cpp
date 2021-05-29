#include "Preview.h"

#include <Context/IAttributesListener.h>

class Preview::AttributesListener final : public IAttributesListener
{
	Preview* const preview;

public:
	AttributesListener(Preview* const preview) : preview(preview)
	{
	}

	void OnUpdate(const int str, const int dex, const int int_, const int fth, const int lck) override
	{
	}
};

Preview::Preview(wxWindow* parent)
	: Title(parent, "Weapon Preview")
	, attributesListener(std::make_shared<AttributesListener>(this))
{
	attributesListener->Register();
}
