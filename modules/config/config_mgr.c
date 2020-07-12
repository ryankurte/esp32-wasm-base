
#include "config_mgr.h"

#include <string.h>


#include "esp_log.h"
#include "nvs_flash.h"


#define TAG "CFG_MGR"


ESP_EVENT_DEFINE_BASE(CONFIG_MGR_EVENT_BASE);


static nvs_handle_t handle = 0;

static void config_mgr_event_handler(void *arg, esp_event_base_t event_base,
        int32_t event_id, void *event_data) {

    // TODO: handle config set events



}

// Initialise config manager
int CONFIG_MGR_init() {
    ESP_LOGI(TAG, "Initialising configuration manager");
    
    // Open NVS
    int res = nvs_open(CONFIG_MGR_PARTITION, NVS_READWRITE, &handle);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS");
        return res;
    }

    // Register event handler
    ESP_ERROR_CHECK(esp_event_handler_register(CONFIG_MGR_EVENT_BASE, ESP_EVENT_ANY_ID, &config_mgr_event_handler, NULL));

    return ESP_OK;
}

// Set a configuration value
int CONFIG_MGR_set(const char* key, const char* val) {
    // Set string in store
    int res = nvs_set_str(handle, key, val);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error setting config: %d", res);
        return res;
    }

    // Emit update event for consuming modules
    config_manager_update_t update = { 0 };
    strncpy(update.key, key, CONFIG_MGR_MAX_KEY);
    strncpy(update.val, val, CONFIG_MGR_MAX_VAL);
    esp_event_post(CONFIG_MGR_EVENT_BASE, CONFIG_MGR_EVENT_CHANGED, &update, sizeof(update),  100 * portTICK_PERIOD_MS);

    // Commit changes
    res = nvs_commit(handle);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error committing config to NVS: %d", res);
        return res;
    }

    return ESP_OK;
}

// Clear a configuration value
int CONFIG_MGR_clear(const char* key) {
    int res = nvs_erase_key(handle, key);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error erasing config: %d", res);
        return res;
    }

    // Emit update event for consuming modules
    config_manager_update_t update = { 0 };
    strncpy(update.key, key, CONFIG_MGR_MAX_KEY);
    esp_event_post(CONFIG_MGR_EVENT_BASE, CONFIG_MGR_EVENT_CHANGED, &update, sizeof(update),  100 * portTICK_PERIOD_MS);


    // Commit changes
    res = nvs_commit(handle);
    if (res != ESP_OK) {
        ESP_LOGE(TAG, "Error committing config to NVS: %d", res);
        return res;
    }

    return ESP_OK;
}

int CONFIG_MGR_done() {

    // Post done event
    esp_event_post(CONFIG_MGR_EVENT_BASE, CONFIG_MGR_EVENT_DONE, NULL, 0,  100 * portTICK_PERIOD_MS);

    return ESP_OK;
}

// Fetch a configuration value
int CONFIG_MGR_get(const char* key, char* val, size_t val_len) {
    return nvs_get_str(handle, key, val, &val_len);
}

int CONFIG_MGR_list() {
    printf("Configuraton values:\n");

    nvs_iterator_t it = nvs_entry_find("nvs", NULL, NVS_TYPE_STR);
    
    while (it != NULL) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);

        char buff[CONFIG_MGR_MAX_VAL] = { 0 };
        CONFIG_MGR_get(info.key, buff, sizeof(buff));

        printf("key: '%s', val: '%s' \n", info.key, buff);

        it = nvs_entry_next(it);
    };

    return 0;
}
