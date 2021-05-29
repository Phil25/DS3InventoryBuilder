#pragma once

#include <Panels/Title.h>

class Preview final : public Title
{
	class AttributesListener;
	std::shared_ptr<AttributesListener> attributesListener;

public:
	Preview(wxWindow* parent);
};