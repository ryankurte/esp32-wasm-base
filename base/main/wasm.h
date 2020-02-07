
#ifndef WASM_APP_H_
#define WASM_APP_H_

#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



#define TASK_NAME_MAX_LEN   32
#define TASK_MAX_ARGS       6
#define TASK_MAX_ARGLEN     16

typedef struct  {
    // Task name
    char        name[TASK_NAME_MAX_LEN];

    // WASM binary data
    uint8_t     *data;
    uint32_t    data_len;

    // Arguments to be passed to the runtime
    char     args[TASK_MAX_ARGS][TASK_MAX_ARGLEN];
    uint32_t arg_count;

    // Thread handle for running task
    TaskHandle_t handle;

    bool        running;

} WasmTask_t;

int WASM_launch_task(const WasmTask_t* wasmInfo); 

int WASM_end_task(const WasmTask_t* wasmInfo);

#endif
