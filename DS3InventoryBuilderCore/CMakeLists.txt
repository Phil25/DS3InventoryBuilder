add_library(DS3InventoryBuilderCore STATIC)

target_link_libraries(DS3InventoryBuilderCore 
	CONAN_PKG::rapidjson
	CONAN_PKG::bzip2)

add_subdirectory("Source")