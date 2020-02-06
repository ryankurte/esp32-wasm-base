
#ifndef APP_MGR_H
#define APP_MGR_H

#include "esp_http_server.h"

// Initialise application manager
int APP_MGR_init();

int APP_MGR_load(char* name, char* file);
int APP_MGR_start();
int APP_MGR_stop();
int APP_MGR_unload();

// Bind application manager console commands
void APP_MGR_register_commands();

// Bind application manager HTTP api
void APP_MGR_register_http(httpd_handle_t server);

#endif
