
#include "wifi_mgr.h"

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "tcpip_adapter.h"
#include "esp_event.h"
#include "lwip/ip4.h"

#define TAG "WIFI"
#define JOIN_TIMEOUT_MS (10000)

ESP_EVENT_DEFINE_BASE(WIFI_MGR_EVENT_BASE);

static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

typedef enum {
    WIFI_DISCONNECTED,
    WIFI_CONNECTED,
} wifi_status_e;

static wifi_status_e wifi_status = WIFI_DISCONNECTED;

static ip4_addr_t ip;
static ip4_addr_t netmask;
static ip4_addr_t gw;

static bool wifi_join(const char *ssid, const char *pass, int timeout_ms);

// Event handler for wifi status changes
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);

        // Retry connection 
        if (wifi_status == WIFI_CONNECTED) {
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
            ESP_LOGI(TAG, "retry to connect to the AP");

        } else {
            wifi_status = WIFI_DISCONNECTED;
        }
        
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        wifi_status = WIFI_CONNECTED;

        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ip = event->ip_info.ip;
        netmask = event->ip_info.netmask;
        gw = event->ip_info.gw;

    } else if (event_base == WIFI_MGR_EVENT_BASE && event_id == WIFI_MGR_CONNECT) {
        wifi_mgr_connect_t* connect = (wifi_mgr_connect_t*) event_data;

        // Run join function
        wifi_join(connect->ssid, connect->pass, 1000);

    } else if (event_base == WIFI_MGR_EVENT_BASE && event_id == WIFI_MGR_DISCONNECT) {
        
        // Run disconnect function
        wifi_status = WIFI_DISCONNECTED;
        ESP_ERROR_CHECK( esp_wifi_disconnect() );

    } else if (event_base == WIFI_MGR_EVENT_BASE && event_id == WIFI_MGR_GET_STATUS) {
        // TODO, if this is required

    }
}


// Initialise WiFi manager
void WIFI_MGR_init() {
    esp_log_level_set("wifi", ESP_LOG_WARN);

    static bool initialized = false;
    if (initialized) {
        return;
    }

    wifi_event_group = xEventGroupCreate();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.nvs_enable = true;
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    
    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_MGR_EVENT_BASE, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL) );

    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_FLASH) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_NULL) );
    ESP_ERROR_CHECK( esp_wifi_start() );

    // TODO: check if wifi config already exists

    wifi_config_t config;
    uint32_t res = esp_wifi_get_config(ESP_IF_WIFI_STA, &config);
    if (res == ESP_OK) {
        ESP_LOGI(TAG, "Found existing wifi configuration (ssid: %s), connecting", config.sta.ssid);

        ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
        ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &config) );
        ESP_ERROR_CHECK( esp_wifi_connect() );
    }

    initialized = true;
}

// Join function connects to the provided wifi network
static bool wifi_join(const char *ssid, const char *pass, int timeout_ms)
{
    wifi_config_t wifi_config = { 0 };
    strlcpy((char *) wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    if (pass) {
        strlcpy((char *) wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));
    }

    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_connect() );

    return 0;
}

/** Arguments used by 'join' function */
static struct {
    struct arg_int *timeout;
    struct arg_str *ssid;
    struct arg_str *password;
    struct arg_end *end;
} connect_args;

// WiFi Status command for CLI
static int wifi_status_cmd(int argc, char **argv) {
    if (wifi_status == WIFI_CONNECTED) {
        ESP_LOGI(TAG, "connected");
        ESP_LOGI(TAG, "ip:%s", ip4addr_ntoa(&ip));
        ESP_LOGI(TAG, "gw:%s", ip4addr_ntoa(&gw));
        ESP_LOGI(TAG, "nm:%s", ip4addr_ntoa(&netmask));
    } else {
        ESP_LOGI(TAG, "disconnected");
    }

    return 0;
}

// WiFi Connect command for CLI
static int wifi_connect_cmd(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &connect_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, connect_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(__func__, "Connecting to '%s'",
             connect_args.ssid->sval[0]);

    // Set default timeout value
    if (connect_args.timeout->count == 0) {
        connect_args.timeout->ival[0] = JOIN_TIMEOUT_MS;
    }

    // Issue connect event
    wifi_mgr_connect_t connect;
    strncpy(connect.ssid, connect_args.ssid->sval[0], sizeof(connect.ssid));
    strncpy(connect.pass, connect_args.password->sval[0], sizeof(connect.pass));

    esp_event_post(WIFI_MGR_EVENT_BASE, WIFI_MGR_CONNECT, &connect, sizeof(wifi_mgr_connect_t), 
        connect_args.timeout->ival[0] / portTICK_PERIOD_MS);

    // Await connection
    int bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT,
                                   pdFALSE, pdTRUE, connect_args.timeout->ival[0] / portTICK_PERIOD_MS);

    if ((bits & WIFI_CONNECTED_BIT) == 0) {
        ESP_LOGW(__func__, "Connection timed out");
        return 1;
    }
    ESP_LOGI(__func__, "Connected");
    return 0;
}

// WiFi disconnect command for CLI
static int wifi_disconnect_cmd(int argc, char **argv) {
    // Post disconnect event
    esp_event_post(WIFI_MGR_EVENT_BASE, WIFI_MGR_DISCONNECT, NULL, 0, 
        1000 / portTICK_PERIOD_MS);

    return 0;
}

void WIFI_MGR_register_commands() {
    connect_args.timeout = arg_int0(NULL, "timeout", "<t>", "Connection timeout, ms");
    connect_args.ssid = arg_str1(NULL, NULL, "<ssid>", "SSID of AP");
    connect_args.password = arg_str0(NULL, NULL, "<pass>", "PSK of AP");
    connect_args.end = arg_end(2);

    const esp_console_cmd_t wifi_status = {
        .command = "wifi-status",
        .help = "Report current WiFi status",
        .hint = NULL,
        .func = &wifi_status_cmd,
        .argtable = NULL,
    };

    const esp_console_cmd_t wifi_join = {
        .command = "wifi-connect",
        .help = "Connect to a WiFi network",
        .hint = NULL,
        .func = &wifi_connect_cmd,
        .argtable = &connect_args
    };

    const esp_console_cmd_t wifi_disconnect = {
        .command = "wifi-disconnect",
        .help = "Disconnect from wifi",
        .hint = NULL,
        .func = &wifi_disconnect_cmd,
        .argtable = NULL,
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&wifi_status) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&wifi_join) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&wifi_disconnect) );
}
