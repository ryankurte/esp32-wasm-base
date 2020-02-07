
#include "wasm.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"

#include "esp_system.h"
#include "esp_log.h"

#include "wasm3.h"
#include "m3_env.h"
#include "m3_api_defs.h"
#include "m3_api_wasi.h"
#include "m3_env.h"
#include "m3_exception.h"
#include "extra/wasi_core.h"

#include "i2c_mgr.h"


#define TAG "WASM"

// Seems we need quite a lot of stack for this task...
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
    m3ApiGetArg      (uint32_t*, ticks_ms_offset)

    // Check args are valid
    if (runtime == NULL || ticks_ms_offset == NULL) { m3ApiReturn(__WASI_EINVAL); }

    // Fetch tick counter
    uint32_t* ticks = (uint32_t*) m3ApiOffsetToPtr(ticks_ms_offset);
    
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    *ticks = (int32_t)tv_now.tv_sec * 1000L + (int32_t)tv_now.tv_usec / 1000;

    m3ApiReturn(__WASI_ESUCCESS);
}

m3ApiRawFunction(m3_i2c_init)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (uint32_t, i2c_port)
    m3ApiGetArg      (uint32_t, freq)
    m3ApiGetArg      (uint32_t, sda)
    m3ApiGetArg      (uint32_t, scl)

    // Check args are valid
    if (runtime == NULL) { m3ApiReturn(__WASI_EINVAL); }

    ESP_LOGI(TAG, "I2C init port: %d freq: %d sda: %d scl: %d\r\n", i2c_port, freq, sda, scl);

    int32_t res = i2c_init(i2c_port, freq, sda, scl);

    m3ApiReturn(res);
}


m3ApiRawFunction(m3_i2c_deinit)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (uint32_t, i2c_port)

    // Check args are valid
    if (runtime == NULL) { m3ApiReturn(__WASI_EINVAL); }

    ESP_LOGI(TAG, "I2C deinit port: %d\r\n", i2c_port);

    int32_t res = i2c_deinit(i2c_port);

    m3ApiReturn(res);
}

m3ApiRawFunction(m3_i2c_write)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (uint32_t, i2c_port)
    m3ApiGetArg      (uint32_t, address)
    m3ApiGetArgMem   (uint8_t*, data_out)
    m3ApiGetArg      (uint32_t, data_out_len)


    // Check args are valid
    if (runtime == NULL) { m3ApiReturn(__WASI_EINVAL); }

    ESP_LOGI(TAG, "I2C write port: %d addr: %x, %p %d bytes\r\n", i2c_port, address, data_out, data_out_len);

    int32_t res = i2c_write(i2c_port, address, data_out, data_out_len);

    m3ApiReturn(res);
}

m3ApiRawFunction(m3_i2c_read)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (uint32_t, i2c_port)
    m3ApiGetArg      (uint32_t, address)
    m3ApiGetArgMem   (uint8_t*, data_in)
    m3ApiGetArg      (uint32_t, data_in_len)


    // Check args are valid
    if (runtime == NULL) { m3ApiReturn(__WASI_EINVAL); }

    ESP_LOGI(TAG, "I2C read port: %d addr: %x, %p %d bytes\r\n", i2c_port, address, data_in, data_in_len);

    int32_t res = i2c_write(i2c_port, address, data_in, data_in_len);

    m3ApiReturn(res);
}

m3ApiRawFunction(m3_i2c_write_read)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (uint32_t, i2c_port)
    m3ApiGetArg      (uint32_t, address)
    m3ApiGetArgMem   (uint8_t*, data_out)
    m3ApiGetArg      (uint32_t, data_out_len)
    m3ApiGetArgMem   (uint8_t*, data_in)
    m3ApiGetArg      (uint32_t, data_in_len)


    // Check args are valid
    if (runtime == NULL) { m3ApiReturn(__WASI_EINVAL); }

    ESP_LOGI(TAG, "I2C write_read port: %d addr: %x, out: %p %d bytes, in: %p %d bytes\r\n", i2c_port, address, data_out, data_out_len, data_in, data_in_len);

    int32_t res = i2c_write_read(i2c_port, address, data_out, data_out_len, data_in, data_in_len);

    m3ApiReturn(res);
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
    m3_LinkRawFunction (module, idk, "log_write", "i(*i)", &m3_log_write);
#if 1
    m3_LinkRawFunction (module, idk, "delay_ms", "i(i)", &m3_delay_ms);
    m3_LinkRawFunction (module, idk, "get_ticks", "i(*)", &m3_get_tick);

    m3_LinkRawFunction (module, idk, "i2c_init", "i(iiii)", &m3_i2c_init);
    m3_LinkRawFunction (module, idk, "i2c_deinit", "i(i)", &m3_i2c_deinit);
    m3_LinkRawFunction (module, idk, "i2c_write", "i(ii*i)", &m3_i2c_write);
    m3_LinkRawFunction (module, idk, "i2c_read", "i(ii*i)", &m3_i2c_read);
    m3_LinkRawFunction (module, idk, "i2c_write_read", "i(ii*i*i)", &m3_i2c_write_read);
#endif

    IM3Function f;
    result = m3_FindFunction (&f, runtime, "main");
    if (result) {
        ESP_LOGI(TAG, "FindFunction: %s", result);
        wasm_res = -6;

        goto teardown_start;
    }

    const char* i_argv[2] = { "1", "test.wasm", NULL };
    result = m3_CallWithArgs (f, 2, i_argv);
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

