
#ifndef FS_MGR_H
#define FS_MGR_H

#include "esp_http_server.h"

// Initialise the file system manager
int FS_MGR_init();

// Write a file to the file system
int FS_MGR_write(char* name, char* data, uint32_t data_len);

// Read a file from the file system
// This allocates data into buff that must be freed when done
int FS_MGR_read(char* name, char** buff, uint32_t *len);

// List files in the file system
int FS_MGR_list(char* dir_name, char* buff, uint32_t buff_len, bool format);

// Register FS CLI commands
void FS_MGR_register_commands();

// Register FS HTTP endpoints
void FS_MGR_register_http(httpd_handle_t server);

#endif
