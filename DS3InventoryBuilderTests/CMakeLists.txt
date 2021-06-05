add_executable(DS3InventoryBuilderTest)

target_link_libraries(DS3InventoryBuilderTest PRIVATE DS3InventoryBuilderCore CONAN_PKG::gtest)

if (WIN32)
	target_compile_options(DS3InventoryBuilderTest PRIVATE "/bigobj")
endif ()

add_subdirectory("Source")