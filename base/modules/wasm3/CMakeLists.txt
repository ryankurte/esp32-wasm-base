
file(GLOB_RECURSE M3_SOURCES "${CMAKE_CURRENT_LIST_DIR}/wasm3/source/*.c")

set(WASI_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/wasm3/platforms/esp32-idf-wasi/main/m3_api_esp_wasi.c
)

idf_component_register(SRCS ${M3_SOURCES}
                    INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/wasm3/source)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error -O3 -DESP32 -DM3_IN_IRAM -Dd_m3MaxFunctionStackHeight=256 -Dd_m3LogOutput=false)

# Disable harmless warnings
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter -Wno-missing-field-initializers)
