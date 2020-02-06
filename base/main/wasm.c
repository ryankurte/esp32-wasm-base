
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


#define TAG "WASM"

#define STACK_SIZE  (32 * 1024)

void vWasmTask( void * pvParameters );
int wasm_run(char* name, uint8_t* wasm, uint32_t wasm_len);

int WASM_launch_task(const WasmTask_t* wasmTask) {
    // Launch task
    xTaskCreate( vWasmTask, wasmTask->name, STACK_SIZE, wasmTask, tskIDLE_PRIORITY, &wasmTask->handle );
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

    if (wasmTask->running) {
        vTaskDelete(wasmTask->handle);
    }

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

    wasmTask->running = true;

    wasm_run(wasmTask->name, wasmTask->data, wasmTask->data_len);

    ESP_LOGI(TAG, "Finished WASM task: %s\r\n", wasmTask->name);

    wasmTask->running = false;

    vTaskDelete(NULL);
}


int wasm_run(char* name, uint8_t* wasm, uint32_t wasm_len) {
    int wasm_res = 0;

    M3Result result;

    ESP_LOGI(TAG, "Loading WebAssembly (mod: %s, p: %p, %d bytes)...\n", name, (void*)wasm, wasm_len);
    IM3Environment env = m3_NewEnvironment ();
        if (env == NULL) {
        ESP_LOGI(TAG, "NewEnvironment failed");
        wasm_res = -1;

        goto teardown_env;
    }

    IM3Runtime runtime = m3_NewRuntime (env, 32 * 1024, NULL);
    if (runtime == NULL) {
        ESP_LOGI(TAG, "NewRuntime failed");
        wasm_res = -2;

        goto teardown_start;
    }

    IM3Module module;
    result = m3_ParseModule (env, &module, wasm, wasm_len);
    if (result) {
        ESP_LOGI(TAG, "ParseModule: %s", result);
        wasm_res = -3;

        // Only unloaded modules should be manually freed
        m3_FreeModule(module);

        goto teardown_start;
    }

    result = m3_LoadModule (runtime, module);
    if (result) {
        ESP_LOGI(TAG, "LoadModule: %s", result);
        wasm_res = -4;

        goto teardown_start;
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
        ESP_LOGI(TAG, "LinkRawFunction 0: %s", result);
        wasm_res = -5;

        goto teardown_start;
    }
#if 0
    result = m3_LinkRawFunction (module, idk, "delay_ms", "i(i)", &m3_delay_ms);
    if (result) {
        ESP_LOGI(TAG, "LinkRawFunction: %s", result);
        return -5;
    }
#endif

    IM3Function f;
    result = m3_FindFunction (&f, runtime, "_start");
    if (result) {
        ESP_LOGI(TAG, "FindFunction: %s", result);
        wasm_res = -6;

        goto teardown_start;
    }

    const char* i_argv[2] = { "test.wasm", NULL };
    result = m3_CallWithArgs (f, 1, i_argv);
    if (result) {
        ESP_LOGI(TAG, "CallWithArgs: %s", result);
        wasm_res = -7;

        goto teardown_start;
    } 

teardown_start:
    m3_FreeRuntime(runtime);

teardown_env:
    m3_FreeEnvironment(env);
    
    return wasm_res;
}

