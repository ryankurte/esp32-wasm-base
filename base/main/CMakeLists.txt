
idf_component_register(
    SRCS "main.c" "console.c" "wasm.c" "wifi_mgr.c" "mqtt_mgr.c" "fs_mgr.c" "app_mgr.c"
    INCLUDE_DIRS ""
    REQUIRES wasm3 console spi_flash nvs_flash mqtt spiffs esp_http_server
    LDFRAGMENTS linker.lf
) 

