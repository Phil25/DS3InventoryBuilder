#pragma once

#include <Database.h>
#include <Frames/FrameMain.h>
#include <wx/wx.h>

class AppMain final : public wxApp
{
	class Images final
	{
		struct ImageData final
		{
			wxImage image;
			wxBitmap bitmap;
		};

		std::map<std::string, ImageData> images;

	public:
		Images(const invbuilder::Database&);

		auto Get(const char* name, const int size) -> const wxBitmap&;
	};

	const invbuilder::Database database;
	std::unique_ptr<Images> images;

	FrameMain* frameMain{nullptr};

public:
	AppMain();

	bool OnInit() override;

	auto GetDatabase() -> const invbuilder::Database&;
	auto GetImage(const char* name, const int size) -> const wxBitmap&;
};

DECLARE_APP(AppMain)
