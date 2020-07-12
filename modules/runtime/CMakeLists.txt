
idf_component_register(
    SRCS "runtime.c" "gpio_mgr.c" "i2c_mgr.c" "spi_mgr.c"
    INCLUDE_DIRS "."
    REQUIRES console wasm3
) 

