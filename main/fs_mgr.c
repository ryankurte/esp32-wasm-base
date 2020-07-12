

#include "fs_mgr.h"

#include <dirent.h>

#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_spiffs.h"
#include "rom/crc.h"

static const char* TAG = "FS_MGR";

int FS_MGR_init() {
    ESP_LOGI(TAG, "Initialising FileSystem Manager");

    // Build SPIFFS config
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 10,
      .format_if_mount_failed = true
    };

    // Register file system
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
            return -1;
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
            return -2;
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
            return -3;
        }
        return -4;
    }

    // Fetch file system info
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    return 0;
}

// Write to a file
int FS_MGR_write(char* name, char* data, uint32_t data_len) {

    ESP_LOGI(TAG, "Opening file %s", name);

    uint32_t crc = crc32_le(0, (uint8_t*) data, data_len);

    FILE* f = fopen(name, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return -1;
    }

    fwrite(data, 1, data_len, f);
    
    fclose(f);
    
    ESP_LOGI(TAG, "File written (%d bytes, crc: %08x)", data_len, crc);

    return 0;
}

// Read from a file, buff will be malloc'd internally and must be
// freed by the caller
int FS_MGR_read(char* name, char** buff, uint32_t *len) {
    uint32_t res;

    ESP_LOGI(TAG, "Reading file %s", name);

    FILE* f = fopen(name, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return -1;
    }

    // Fetch the file size
    fseek(f, 0L, SEEK_END);
    *len = ftell(f);
    rewind(f);
    ESP_LOGI(TAG, "File: %s, reading %d bytes", name, *len);

    *buff = malloc(*len);
    if (*buff == NULL) {
        ESP_LOGE(TAG, "Failed to allocate data for reading");
        fclose(f);
        return -2;
    }

    // Read file data
    res = fread(*buff, 1, *len, f);
    if (res != *len) {
        ESP_LOGE(TAG, "Read res: %d len: %d", res, *len);
    }

    uint32_t crc = crc32_le(0, (uint8_t*) *buff, *len);
    ESP_LOGI(TAG, "Read file (CRC: 0x%08x)", crc);

    // Close file
    fclose(f);

    // Return data
    return 0;
}

// List files in filesystem
int FS_MGR_list(char* dir_name, char* buff, uint32_t buff_len, bool format) {
     ESP_LOGI(TAG, "Reading dir %s", dir_name);

    // List running tasks
    DIR *d;
    struct dirent *dir;
    uint32_t c = 0;

    d = opendir(dir_name);
    if (d == NULL) {
        return -1;
    }

    if(format) {
        c += snprintf(buff+c, buff_len-c, "[\r\n");
    }

    while ((dir = readdir(d)) != NULL) {
        if (!format) {
            c += snprintf(buff+c, buff_len-c, "%s\r\n", dir->d_name);
        } else {
            c += snprintf(buff+c, buff_len-c, "\t%s,\r\n", dir->d_name);
        }
    }
    closedir(d);

    if(format) {
        c += snprintf(buff+c, buff_len-c, "]");
    }

    return c;
}


int FS_MGR_delete(char* file_name) {
    return remove(file_name);
}

// Helper to load query params from a request
bool get_query_param(httpd_req_t *req, char* name, char* value, uint32_t value_len) {
    size_t buf_len;

    bool found = false;
    buf_len = httpd_req_get_url_query_len(req) + 1;

    if (buf_len > 1) {
        char* buf = malloc(buf_len);

        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {

            ESP_LOGI(TAG, "Found URL query => %s, searching for %s", buf, name);

            /* Get value of expected key from query string */
            uint32_t res = httpd_query_key_value(buf, name, value, value_len);
            if (res == ESP_OK) {
                ESP_LOGI(TAG, "Found URL query parameter => query =%s", value);
                found = true;

            } else if (res == ESP_ERR_HTTPD_RESULT_TRUNC) {
                ESP_LOGI(TAG, "URL query parameter %s truncated", name);
            } else if (res == ESP_ERR_INVALID_ARG) {
                ESP_LOGI(TAG, "URL query parameter %s invalid args", name);
            }
        }

        free(buf);
    }

    return found;
}

esp_err_t file_get_handler(httpd_req_t *req)
{
    char name_buff[32];

    // Fetch file name param
    if( get_query_param(req, "file", name_buff, sizeof(name_buff)) ) {
        // Return file
        char* data;
        uint32_t len;

        if (FS_MGR_read(name_buff, &data, &len) < 0) {
            httpd_resp_send_err(req, 500, "Error reading file");
            return ESP_OK;
        }

        httpd_resp_send(req, data, len);

        free(data);

    } else if( get_query_param(req, "dir", name_buff, sizeof(name_buff)) ) {
        // List files
        char files[256];

        int c = FS_MGR_list(name_buff, files, sizeof(files), true);
        if (c < 0) {
            httpd_resp_send_err(req, 500, "Error reading dir");
            return ESP_OK;
        }

        httpd_resp_send(req, files, c);
    } else {
        httpd_resp_send_err(req, 500, "file or dir arguments required");
    }

    return ESP_OK;
}

esp_err_t file_post_handler(httpd_req_t *req)
{
    char file_name[32];

    // Check content length is sane
    if (req->content_len > (100 * 1024)) {
        httpd_resp_send_err(req, 400, "Content length exceeds maximum");
        return ESP_OK;
    }

    bool has_name = get_query_param(req, "file", file_name, sizeof(file_name));
    if (!has_name) {
        httpd_resp_send_err(req, 400, "File name parameter required");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Receiving file %s with content length %d", file_name, req->content_len);

    // Allocate space for recieving object
    char* content = malloc(req->content_len);
    if (content == NULL) {
        httpd_resp_send_err(req, 500, "Error allocating receive buffer");
        return ESP_OK;
    }
    memset(content, 0xFF, req->content_len);
    
    // Receive data
    uint32_t c = 0;
    while (c < req->content_len) {
        int ret = httpd_req_recv(req, content + c, req->content_len - c);

        if (ret < 0) {
            ESP_LOGI(TAG, "HTTP receive error: %d", ret);
            httpd_resp_send_err(req, 500, "Receive error");

            goto post_done;
        }

        c += ret;
    }

    

    // Write data to FS
    FS_MGR_write(file_name, content, req->content_len);

    // Respond with OK
    const char resp[] = "OK\r\n";
    httpd_resp_send(req, resp, strlen(resp));

post_done:

    // Free data storage
    free(content);

    return ESP_OK;
}

httpd_uri_t fs_uri_get = {
    .uri      = "/fs",
    .method   = HTTP_GET,
    .handler  = file_get_handler,
    .user_ctx = NULL
};

httpd_uri_t fs_uri_post = {
    .uri      = "/fs",
    .method   = HTTP_POST,
    .handler  = file_post_handler,
    .user_ctx = NULL
};

void FS_MGR_register_http(httpd_handle_t server) {
    httpd_register_uri_handler(server, &fs_uri_get);
    httpd_register_uri_handler(server, &fs_uri_post);
}