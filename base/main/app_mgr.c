

#include "app_mgr.h"

#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "esp_http_server.h"

#include "fs_mgr.h"
#include "wasm.h"


static const char* TAG = "APP_MGR";

#define MAX_WASM_TASKS   4

WasmTask_t *task = NULL;

int APP_MGR_init() {
    ESP_LOGI(TAG, "Initialising Application Manager");


    return 0;
}

int APP_MGR_status() {
    if (task == NULL) {
        ESP_LOGI(TAG, "No task loaded");
        return 0;
    }

    if (task->running) {
        ESP_LOGI(TAG, "Running task: %s\r\n", task->name);
    } else {
        ESP_LOGI(TAG, "Loaded task: %s (not running)\r\n", task->name);
    }
    
    return 0;
}

int APP_MGR_load(char* name, char* file) {
    int res;

    ESP_LOGI(TAG, "Loading task: %s from file: %s", name, file);

    if (task != NULL) {
        ESP_LOGI(TAG, "Task already loaded, first unload existing task");
        return -1;
    }

    task = malloc(sizeof(WasmTask_t));
    if (task == NULL) {
        ESP_LOGI(TAG, "Error allocating memory for task");
        return -2;
    }

    memset(task, 0, sizeof(WasmTask_t));

    // Set task name
    strncpy(task->name, name, TASK_NAME_MAX_LEN);

    // Load task into memory
    uint8_t* buff;
    res = FS_MGR_read(file, &buff, &task->data_len);
    if (res < 0) {
        ESP_LOGI(TAG, "Error %d loading file %s", res, file);
        return -3;
    }
    task->data = buff;

    ESP_LOGI(TAG, "Task %s loaded", task->name);

    return 0;
}

int APP_MGR_start() {
    if (task == NULL) {
        ESP_LOGI(TAG, "No task loaded");
        return -1;
    }

    if (task->running) {
        ESP_LOGI(TAG, "Task %s already running", task->name);
        return -2;
    }

    // Launch task
    int res = WASM_launch_task(task);
    if (res < 0) {
        ESP_LOGI(TAG, "Error %d launching task %s", res, task->name);
        return -3;
    }

    return 0;
}

int APP_MGR_stop() {
    int res;

    ESP_LOGI(TAG, "Stopping task");

    if (task == NULL) {
        ESP_LOGI(TAG, "Task not loaded");
        return -1;
    }

    if (!task->running) {
        ESP_LOGI(TAG, "Task %s not running", task->name);
        return -2;
    }

    // Stop task
    res = WASM_end_task(task);
    if (res < 0) {
        ESP_LOGI(TAG, "Error %d stopping task %s", res, task->name);
    }

    return 0;
}

int APP_MGR_unload() {

    ESP_LOGI(TAG, "Unloading task");

    if (task == NULL) {
        ESP_LOGI(TAG, "Task not loaded");
        return -1;
    }

    if (task->running) {
        ESP_LOGI(TAG, "Task %s running, stop task before unloading", task->name);
        return -2;
    }

    // De-allocate memory
    free(task->data);
    free(task);

    ESP_LOGI(TAG, "Task unloaded");

    return 0;
}



// App Status command for CLI
static int task_status_cmd(int argc, char **argv) {
    APP_MGR_status();

    return 0;
}

static struct {;
    struct arg_str *name;
    struct arg_str *file;
    struct arg_end *end;
} load_args;


// App load command for CLI
static int task_load_command(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &load_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, load_args.end, argv[0]);
        return 1;
    }

    int res = APP_MGR_load((char*) load_args.name->sval[0], (char*) load_args.file->sval[0]);
    if (res < 0) {
        ESP_LOGI(TAG, "Error %d loading task", res);
        return res;
    }

    return 0;
}

static int task_launch_command(int argc, char **argv) {
    int res;

    int nerrors = arg_parse(argc, argv, (void **) &load_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, load_args.end, argv[0]);
        return 1;
    }

    res = APP_MGR_load((char*) load_args.name->sval[0], (char*) load_args.file->sval[0]);
    if (res < 0) {
        ESP_LOGI(TAG, "Error %d loading task", res);
        return res;
    }

    res = APP_MGR_start();
    if (res < 0) {
        ESP_LOGI(TAG, "Error %d loading task", res);
        return res;
    }

    return 0;
}

static int task_start_command(int argc, char **argv) {
    return APP_MGR_start();
}

static int task_stop_command(int argc, char **argv) {
    return APP_MGR_stop();
}

static int task_unload_command(int argc, char **argv) {
    return APP_MGR_unload();
}

void APP_MGR_register_commands() {
    load_args.name = arg_str0(NULL, NULL, "<name>", "Name of task to launch");
    load_args.file = arg_str1(NULL, NULL, "<file>", "File to load task from");
    load_args.end = arg_end(2);

    const esp_console_cmd_t task_status = {
        .command = "task-status",
        .help = "Report current task status",
        .hint = NULL,
        .func = &task_status_cmd,
        .argtable = NULL,
    };

    const esp_console_cmd_t task_launch = {
        .command = "task-launch",
        .help = "Initialise and run a new task from the provided file",
        .hint = NULL,
        .func = &task_launch_command,
        .argtable = &load_args
    };

    const esp_console_cmd_t task_load = {
        .command = "task-load",
        .help = "Initialise a new task from the provided file",
        .hint = NULL,
        .func = &task_load_command,
        .argtable = &load_args
    };

    const esp_console_cmd_t task_start = {
        .command = "task-start",
        .help = "Start the loaded task",
        .hint = NULL,
        .func = &task_start_command,
        .argtable = NULL,
    };

    const esp_console_cmd_t task_stop = {
        .command = "task-stop",
        .help = "Stop the running task",
        .hint = NULL,
        .func = &task_stop_command,
        .argtable = NULL,
    };

    const esp_console_cmd_t task_unload = {
        .command = "task-unload",
        .help = "Stop the running task",
        .hint = NULL,
        .func = &task_unload_command,
        .argtable = NULL,
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&task_status) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&task_launch) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&task_load) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&task_start) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&task_stop) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&task_unload) );
}
