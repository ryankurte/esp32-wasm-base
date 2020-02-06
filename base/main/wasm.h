
#ifndef WASM_APP_H_
#define WASM_APP_H_

#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



#define TASK_NAME_MAX_LEN 32


typedef struct  {
    char        name[TASK_NAME_MAX_LEN];

    uint8_t     *data;

    uint32_t    data_len;

    TaskHandle_t handle;

    bool        running;

} WasmTask_t;

int WASM_launch_task(const WasmTask_t* wasmInfo); 

int WASM_end_task(const WasmTask_t* wasmInfo);

#endif
