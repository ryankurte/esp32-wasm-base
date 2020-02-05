
#ifndef WASM_APP_H_
#define WASM_APP_H_

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


typedef struct  {
    char        *name;
    
    uint8_t     *data;

    uint32_t    data_len;

    TaskHandle_t handle;

} WasmTask_t;

int WASM_launch_task(const WasmTask_t* wasmInfo); 

int WASM_end_task(const WasmTask_t* wasmInfo);

#endif
