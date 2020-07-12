/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "nvs_flash.h"
#include "esp_spi_flash.h"
#include "esp_http_server.h"

#include "wifi_mgr.h"
#include "mqtt_mgr.h"
#include "fs_mgr.h"
#include "app_mgr.h"
#include "console.h"


void app_main(void)
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    ESP_ERROR_CHECK( nvs_flash_init() );
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_create_default() );

    // Initialise components

    // HTTP server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    ESP_ERROR_CHECK( httpd_start(&server, &config) );

    CONSOLE_init();

    WIFI_MGR_init();
    WIFI_MGR_register_commands();

    FS_MGR_init();
    FS_MGR_register_http(server);

    APP_MGR_init();
    APP_MGR_register_commands();
    APP_MGR_register_http(server);

    // Run the console
    CONSOLE_run();
}
