#include "InventoryEncoder.h"

#include <AppMain.h>
#include <sstream>
#include <iomanip>
#include <cassert>

namespace
{
	constexpr const char* revision = "1";
	constexpr char delim = '-';

	inline auto Split(std::string str)
	{
		std::stringstream ss{std::move(str)};
		std::string part;
		std::vector<std::string> out;

		while (std::getline(ss, part, delim))
			out.emplace_back(std::move(part));

		return out;
	}

	inline auto HexToInt(const char h)
	{
		switch (h)
		{
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a': case'A': return 10;
		case 'b': case'B': return 11;
		case 'c': case'C': return 12;
		case 'd': case'D': return 13;
		case 'e': case'E': return 14;
		case 'f': case'F': return 15;
		}

		throw inventory_encoder::Exception::Invalid;
		return -1;
	}

	auto Decode_1(const std::vector<std::string>& vec) -> WeaponContext::Vector
	{
		WeaponContext::Vector out;

		for (size_t i = 1; i < vec.size(); ++i)
		{
			auto code = vec[i];
			if (code.length() < 3)
				throw inventory_encoder::Exception::Invalid;

			const auto inf = HexToInt(code[0]);
			const auto lvl = HexToInt(code[1]);
			code[0] = '0';
			code[1] = '0';

			int id = std::stoul(code, nullptr, 16) * 10'000;
			auto name = wxGetApp().GetDatabase().GetWeaponName(id);
			if (name.empty())
				throw inventory_encoder::Exception::Invalid;

			auto context = std::make_shared<WeaponContext>(std::move(name), lvl, inf);
			if (!context->IsValid())
				throw inventory_encoder::Exception::Invalid;

			out.emplace_back(std::move(context));
		}

		return out;
	}
}

auto inventory_encoder::Encode(const WeaponContext::WeakVector& weapons) -> std::string
{
	std::ostringstream oss;

	oss << revision << std::hex;
	for (const auto& weakContext : weapons)
	{
		const auto& context = weakContext.lock();
		if (!context)
		{
			assert(false && "invalid weapon when encoding");
			continue;
		}

		const auto& weapon = wxGetApp().GetDatabase().GetWeapon(context->GetName());
		oss << delim;
		oss << static_cast<int>(context->GetInfusion());
		oss << context->GetLevel();
		oss << (weapon.id / 10'000);
	}

	return oss.str();
}

auto inventory_encoder::Decode(std::string inventoryCode) -> WeaponContext::Vector
{
	if (inventoryCode.empty())
		throw Exception::Empty;

	const auto vec = Split(std::move(inventoryCode));
	if (vec.empty())
		throw Exception::Invalid;

	if (vec[0] == "1")
		return Decode_1(vec);

	throw Exception::RevisionNotSupported;
	return {};
}
