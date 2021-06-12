if (WIN32)
	add_executable(DS3InventoryBuilder WIN32)
elseif (APPLE)
	add_executable(DS3InventoryBuilder MACOSX_BUNDLE)
else ()
	add_executable(DS3InventoryBuilder)
endif ()

set(REG_FILE ".*\.(c[pp]?|h[pp]?)")
source_group("Source" REGULAR_EXPRESSION "${REG_FILE}")
source_group("Source\\Context" REGULAR_EXPRESSION "Context/${REG_FILE}")
source_group("Source\\Frames" REGULAR_EXPRESSION "Frames/${REG_FILE}")
source_group("Source\\Menus" REGULAR_EXPRESSION "Menus/${REG_FILE}")
source_group("Source\\Panels" REGULAR_EXPRESSION "Panels/${REG_FILE}")
source_group("Source\\Utils" REGULAR_EXPRESSION "Utils/${REG_FILE}")

target_link_libraries(DS3InventoryBuilder PRIVATE DS3InventoryBuilderCore CONAN_PKG::wxwidgets CONAN_PKG::fmt)

add_subdirectory("Source")