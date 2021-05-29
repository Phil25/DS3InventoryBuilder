#include "AppMain.h"

#include <wx/mstream.h>

wxIMPLEMENT_APP(AppMain);

AppMain::Images::Images(const invbuilder::Database& db)
{
	unsigned int size;
	std::string name;
	uint8_t* buffer{nullptr};

	while (buffer = db.GetImage(size, name))
	{
		auto stream = wxMemoryInputStream{buffer, size};
		auto data = ImageData{{stream, wxBITMAP_TYPE_PNG}, {}};
		images.emplace(std::move(name), std::move(data));
	}
}

auto AppMain::Images::Get(const std::string& name, const int size) -> const wxBitmap&
{
	auto it = images.find(name);
	assert(it != images.end() && "image not found");

	auto& cache = it->second;

	if (cache.bitmap.GetWidth() != size)
	{
		auto image = cache.image.Scale(size, size, wxIMAGE_QUALITY_BICUBIC);
		cache.bitmap = wxBitmap{std::move(image)};
	}

	assert(cache.bitmap.GetHeight() == size && "images should be 1:1");
	return cache.bitmap;
}

AppMain::AppMain()
	: database(invbuilder::Database::Create())
{
}

bool AppMain::OnInit()
{
	wxInitAllImageHandlers();

	images = std::make_unique<Images>(database);

	auto title = wxString("DS3InventoryBuilder -- v");
	title.Append(APP_VERSION);

	frameMain = new FrameMain(std::move(title));
	frameMain->Show();

	return true;
}

auto AppMain::GetDatabase() -> const invbuilder::Database&
{
	return database;
}

auto AppMain::GetImage(const std::string& name, const int size) -> const wxBitmap&
{
	assert(images && "images not initialized");
	assert(size > 0 && "bitmap size must be positive");
	return images->Get(name, size);
}

auto AppMain::GetSessionData() -> SessionData&
{
	return sessionData;
}
