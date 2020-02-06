
#ifndef WIFI_MGR_H
#define WIFI_MGR_H

#include "esp_event.h"

// Wifi manager event type
ESP_EVENT_DECLARE_BASE(WIFI_MGR_EVENT_BASE);

enum {
    // Issue WiFi connect event
    WIFI_MGR_CONNECT,
    // Issue WiFi disconnect event
    WIFI_MGR_DISCONNECT,
    // Request WiFi status
    WIFI_MGR_GET_STATUS,
    // Wifi Status event
    WIFI_MGR_STATUS,
} wifi_mgr_event_e;

// Based on IDF definitions
#define WIFI_SSID_MAX_LEN 32
#define WIFI_PASS_MAX_LEN 64

// Connect object
typedef struct {
    char ssid[WIFI_SSID_MAX_LEN];
    char pass[WIFI_PASS_MAX_LEN];
} wifi_mgr_connect_t;

// Initialise wifi manager
void WIFI_MGR_init();

// Register wifi commands
void WIFI_MGR_register_commands();

#endif 