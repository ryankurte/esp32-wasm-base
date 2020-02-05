
#include "wasm.h"

#include <stdio.h>
#include <time.h>
#include <unistd.h>


#include "esp_system.h"
#include "esp_log.h"

#include "wasm3.h"
#include "m3_env.h"
#include "m3_api_defs.h"
#include "m3_api_wasi.h"
#include "m3_env.h"
#include "m3_exception.h"
#include "extra/wasi_core.h"


#define TAG "m3"

#define STACK_SIZE  (32 * 1024)

void vWasmTask( void * pvParameters );
int wasm_run(char* name, uint8_t* wasm, uint32_t wasm_len);

int WASM_launch_task(const WasmTask_t* wasmTask) {
    // Launch task
    xTaskCreate( vWasmTask, wasmTask->name, STACK_SIZE, &wasmTask, tskIDLE_PRIORITY, &wasmTask->handle );
    if (wasmTask->handle == NULL) {
        ESP_LOGI(TAG, "Failed to launch WASM task: %s", wasmTask->name);
        return -1;
    }

    return 0;
}

int WASM_end_task(const WasmTask_t* wasmTask) {
    if (wasmTask->handle == NULL) {
        ESP_LOGI(TAG, "Failed to end WASM task %s", wasmTask->name);
        return -1;
    }

    vTaskDelete(wasmTask->handle);

    return 0;
}

// WASM logging function
m3ApiRawFunction(m3_log_write)
{
    // Load arguments
    m3ApiReturnType  (uint32_t)
    m3ApiGetArg      (uint32_t, buff_offset)
    m3ApiGetArg      (uint32_t, buff_len)

    // Check args are valid
    if (runtime == NULL || buff_offset == NULL) { m3ApiReturn(__WASI_EINVAL); }

    char* buff = m3ApiOffsetToPtr(buff_offset);

    //buff[buff_len-1] = '\0';
    fwrite(buff, 1, buff_len, stdout);

    m3ApiReturn(__WASI_ESUCCESS);
}

// WASM logging function
m3ApiRawFunction(m3_delay_ms)
{
    // Load arguments
    m3ApiReturnType  (uint32_t)
    m3ApiGetArg      (uint32_t, delay_ms)

    // Check args are valid
    if (runtime == NULL) { m3ApiReturn(__WASI_EINVAL); }

    // Call delay to yeild this thread
    vTaskDelay(delay_ms / portTICK_PERIOD_MS);

    m3ApiReturn(__WASI_ESUCCESS);
}

// WASM logging function
m3ApiRawFunction(m3_get_tick)
{
    // Load arguments
    m3ApiReturnType  (uint32_t)
    m3ApiGetArg      (uint32_t*, ticks_ms)

    // Check args are valid
    if (runtime == NULL || ticks_ms == NULL) { m3ApiReturn(__WASI_EINVAL); }

    // Fetch tick counter
   // *ticks_ms = getTimeSinceStart();

    m3ApiReturn(__WASI_ESUCCESS);
}

void vWasmTask( void * pvParameters ) {
    WasmTask_t* wasmTask = (WasmTask_t*) pvParameters;

    ESP_LOGI(TAG, "Running WASM task: %s\r\n", wasmTask->name);

    wasm_run(wasmTask->name, wasmTask->data, wasmTask->data_len);

    ESP_LOGI(TAG, "Finished WASM task: %s\r\n", wasmTask->name);
}


int wasm_run(char* name, uint8_t* wasm, uint32_t wasm_len) {
    M3Result result;

    printf("Loading WebAssembly (mod: %s, %d bytes)...\n", name, wasm_len);
    IM3Environment env = m3_NewEnvironment ();
        if (env == NULL) {
        ESP_LOGI(TAG, "NewEnvironment failed");
        return -1;
    }

    IM3Runtime runtime = m3_NewRuntime (env, 32 * 1024, NULL);
    if (runtime == NULL) {
        ESP_LOGI(TAG, "NewRuntime failed");
        return -2;
    }

    IM3Module module;
    result = m3_ParseModule (env, &module, wasm, wasm_len);
    if (result) {
        ESP_LOGI(TAG, "ParseModule: %s", result);
        return -3;
    }

    result = m3_LoadModule (runtime, module);
    if (result) {
        ESP_LOGI(TAG, "LoadModule: %s", result);
        return -4;
    }
#if 0
    result = m3_LinkEspWASI (runtime->modules);
    if (result) {
        ESP_LOGI(TAG, "LinkEspWASI: %s", result);
        return -5;
    }
#endif
    char* idk = "env";
    result = m3_LinkRawFunction (module, idk, "log_write", "i(*i)", &m3_log_write);
    if (result) {
        ESP_LOGI(TAG, "LinkRawFunction: %s", result);
        return -5;
    }

    IM3Function f;
    result = m3_FindFunction (&f, runtime, "_start");
    if (result) {
        ESP_LOGI(TAG, "FindFunction: %s", result);
        return -6;
    }

    printf("Running...\n");

    const char* i_argv[2] = { "test.wasm", NULL };
    result = m3_CallWithArgs (f, 1, i_argv);
    if (result) {
        ESP_LOGI(TAG, "CallWithArgs: %s", result);
        return -7;
    }
    
    return 0;
}

