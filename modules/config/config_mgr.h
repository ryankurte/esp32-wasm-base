
#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stdint.h>

#include "esp_event.h"
#include "nvs_flash.h"


// Configuration manager event type
ESP_EVENT_DECLARE_BASE(CONFIG_MGR_EVENT_BASE);

// Configuration manager events
enum {
    // Issue a configuration update to the config manager
    CONFIG_MGR_EVENT_SET,

    // Receive a configuration change from the config manager
    CONFIG_MGR_EVENT_CHANGED,

    // Indicate a set of configuration changes is complete
    CONFIG_MGR_EVENT_DONE,
} config_mgr_event_e;


#define CONFIG_MGR_MAX_KEY  32
#define CONFIG_MGR_MAX_VAL  256

#define CONFIG_MGR_PARTITION  "cfg"


// Connect object
typedef struct {
    char key[CONFIG_MGR_MAX_KEY];   // Configuration key
    char val[CONFIG_MGR_MAX_VAL];   // Configuration value
} config_manager_update_t;


// Initialise config manager
int CONFIG_MGR_init();

// Register config manager CLI commands
void CONFIG_MGR_register_commands();


// Set a configuration value
int CONFIG_MGR_set(const char* key, const char* val);

// Clear a configuration key
int CONFIG_MGR_clear(const char* key);

// Emit end-of-configuration signal
// (allows re-configuration of multiple fields)
int CONFIG_MGR_done();

// Fetch a configuration value
int CONFIG_MGR_get(const char* key, char* val, size_t val_len);

// List configuration values (to the terminal)
int CONFIG_MGR_list();


#endif
