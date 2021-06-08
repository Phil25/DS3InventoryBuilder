target_sources(DS3InventoryBuilderCore
	PRIVATE
		"Calculator.cpp"
		"Calculator.h"
		"Comparators.cpp"
		"Comparators.h"
		"Database.cpp"
		"Database.h"
		"ImageStorage.cpp"
		"ImageStorage.h"
		"PlayerAttributes.hpp"
		"Weapon.hpp"
)

target_include_directories(DS3InventoryBuilderCore PUBLIC ${CMAKE_CURRENT_LIST_DIR})