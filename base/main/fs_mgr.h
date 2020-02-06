
#ifndef FS_MGR_H
#define FS_MGR_H

#include "esp_http_server.h"

// Initialise the file system manager
int FS_MGR_init();

// Register FS CLI commands
void FS_MGR_register_commands();

// Register FS HTTP endpoints
void FS_MGR_register_http(httpd_handle_t server);

#endif
