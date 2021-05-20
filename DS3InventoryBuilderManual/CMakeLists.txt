add_executable(DS3InventoryBuilderManual)

target_link_libraries(DS3InventoryBuilderManual PRIVATE DS3InventoryBuilderCore)

target_sources(DS3InventoryBuilderManual PRIVATE "main.cpp")