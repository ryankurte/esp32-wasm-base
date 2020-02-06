

#include "app_mgr.h"

#include "esp_log.h"
#include "esp_spiffs.h"

static const char* TAG = "APP_MGR";

int APP_MGR_init() {
    ESP_LOGI(TAG, "Initialising Application Manager");


    return 0;
}

int APP_MGR_write() {

return 0;
}

int APP_MGR_list() {
    // List running tasks
    
    return 0;
}

int APP_MGR_run() {
    // Load task into memory

    // Add to task tracking

    // Launch task
    return 0;
}

int APP_MGR_stop() {
    // Stop task
    
    // Remove task from tracking

    // De-allocate memory
    return 0;
}
