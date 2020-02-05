
idf_component_register(
    SRCS "main.c" "wasm.c"
    INCLUDE_DIRS ""
    REQUIRES wasm3 console spi_flash nvs_flash mqtt
    LDFRAGMENTS linker.lf
) 

