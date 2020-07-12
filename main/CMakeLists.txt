
idf_component_register(
    SRCS "main.c" "console.c" "fs_mgr.c" "app_mgr.c"
    INCLUDE_DIRS ""
    REQUIRES wasm3 console spi_flash nvs_flash spiffs esp_http_server networking comms runtime
    LDFRAGMENTS linker.lf
) 

