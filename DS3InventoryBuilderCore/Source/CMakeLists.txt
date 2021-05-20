target_sources(DS3InventoryBuilderCore
	PRIVATE
		"Database.cpp"
		"Database.h"
		"PlayerAttributes.hpp"
		"Weapon.hpp"
)

target_include_directories(DS3InventoryBuilderCore PUBLIC ${CMAKE_CURRENT_LIST_DIR})