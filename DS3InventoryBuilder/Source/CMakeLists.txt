target_sources(DS3InventoryBuilder
	PRIVATE
		"AppMain.cpp"
		"AppMain.h"
)

target_include_directories(DS3InventoryBuilder PRIVATE ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory("Context")
add_subdirectory("Frames")
add_subdirectory("Menus")
add_subdirectory("Panels")
add_subdirectory("Utils")