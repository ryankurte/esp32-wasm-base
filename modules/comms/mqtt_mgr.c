
#include "mqtt_mgr.h"

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
#include "mqtt_client.h"
#include "esp_tls.h"

#define TAG "MQTT"
#define JOIN_TIMEOUT_MS (10000)

static EventGroupHandle_t mqtt_event_group;
const static int CONNECTED_BIT = BIT0;

typedef enum {
    MQTT_DISCONNECTED,
    MQTT_CONNECTED,
} mqtt_status_e;

volatile static mqtt_status_e mqtt_status = MQTT_DISCONNECTED;

static void mqtt_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data;
	//esp_mqtt_client_handle_t client = event->client;

	switch (event->event_id) {
		case MQTT_EVENT_CONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            mqtt_status = MQTT_CONNECTED;
            xEventGroupSetBits(mqtt_event_group, CONNECTED_BIT);

			break;

		case MQTT_EVENT_DISCONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            mqtt_status = MQTT_CONNECTED;

			break;

		case MQTT_EVENT_ERROR:
			ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
	        int mbedtls_err = 0;
	        esp_err_t err = esp_tls_get_and_clear_last_error(event->error_handle, &mbedtls_err, NULL);
	        ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
	        ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
			break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;

		default:
			// Unhandled
			break;
	}
}

esp_mqtt_client_handle_t client;

void mqtt_connect(const char* url, const char* client_id, const char* username, const char* password, const char* server_cert,
				const char* client_cert_pem, const char* client_key_pem) {

    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = url,
		.client_id = client_id,
		.username = username,
		.password = password,
		.cert_pem = server_cert,
		.client_cert_pem = client_cert_pem,
		.client_key_pem = client_key_pem
	};

    mqtt_event_group = xEventGroupCreate();

    client = esp_mqtt_client_init(&mqtt_cfg);
    
	ESP_ERROR_CHECK( esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client) );
	
    xEventGroupClearBits(mqtt_event_group, CONNECTED_BIT);

    ESP_ERROR_CHECK( esp_mqtt_client_start(client) );

    xEventGroupWaitBits(mqtt_event_group, CONNECTED_BIT, false, true, configTICK_RATE_HZ * 3);
}

static int mqtt_status_cmd(int argc, char **argv) {
    switch (mqtt_status) {
        case MQTT_CONNECTED:
            printf("MQTT connected!\r\n");
            break;
        case MQTT_DISCONNECTED:
            printf("MQTT disconnected\r\n");
            break;
    }
    return ESP_OK;
}

static struct {
    struct arg_str *url;
    struct arg_str *client_id;
    struct arg_str *username;
	struct arg_str *password;
    struct arg_end *end;
} mqtt_connect_args;

static int mqtt_connect_cmd(int argc, char **argv) {
	// Parse arguments fom cli
	int nerrors = arg_parse(argc, argv, (void **) &mqtt_connect_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, mqtt_connect_args.end, argv[0]);
        return 1;
    }

    printf("Connecting to MQTT broker: %s with client_id: %s user: %s pass: %s", mqtt_connect_args.url->sval[0], mqtt_connect_args.client_id->sval[0], mqtt_connect_args.username->sval[0], mqtt_connect_args.password->sval[0]);

    // Call connect method
    mqtt_connect(mqtt_connect_args.url->sval[0], mqtt_connect_args.client_id->sval[0], mqtt_connect_args.username->sval[0], mqtt_connect_args.password->sval[0], 
        NULL, NULL, NULL);


	return ESP_OK;
}

static struct {
    struct arg_str *topic;
    struct arg_str *data;
    struct arg_int *qos;
    struct arg_end *end;
} mqtt_publish_args;

static int mqtt_publish_cmd(int argc, char **argv) {
	// Parse arguments fom cli
	int nerrors = arg_parse(argc, argv, (void **) &mqtt_publish_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, mqtt_connect_args.end, argv[0]);
        return 1;
    }

    printf("MQTT publishing to: %s with data: '%s' and QoS: %d\r\n", mqtt_publish_args.topic->sval[0], mqtt_publish_args.data->sval[0], mqtt_publish_args.qos->ival[0]);


    int msg_id = esp_mqtt_client_publish(client, mqtt_publish_args.topic->sval[0], mqtt_publish_args.data->sval[0], 
            strlen(mqtt_publish_args.data->sval[0]), mqtt_publish_args.qos->ival[0], 0);

    printf("MQTT publish message id: %d\r\n", msg_id);

	return ESP_OK;
}

static struct {
    struct arg_str *topic;
    struct arg_int *qos;
    struct arg_end *end;
} mqtt_subscribe_args;

static int mqtt_subscribe_cmd(int argc, char **argv) {
	// Parse arguments fom cli
	int nerrors = arg_parse(argc, argv, (void **) &mqtt_subscribe_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, mqtt_connect_args.end, argv[0]);
        return 1;
    }

    printf("MQTT subscribing to: %s with QoS: %d\r\n", mqtt_subscribe_args.topic->sval[0], mqtt_subscribe_args.qos->ival[0]);

    esp_mqtt_client_subscribe(client, mqtt_subscribe_args.topic->sval[0], mqtt_subscribe_args.qos->ival[0]);

	return ESP_OK;
}

static int mqtt_stop_cmd(int argc, char **argv) {
	printf("Stopping MQTT client\r\n");

    esp_mqtt_client_stop(client);

	return ESP_OK;
}

void register_mqtt_cmds(void)
{
    
    const esp_console_cmd_t mqtt_status = {
        .command = "mqtt-status",
        .help = "Report current MQTT status",
        .hint = NULL,
        .func = &mqtt_status_cmd,
        .argtable = NULL,
    };

    mqtt_connect_args.url = arg_str0(NULL, NULL, "<url>", "MQTT server URL");
    mqtt_connect_args.client_id = arg_str0("c", "client-id", "<client_id>", "MQTT client id");
    mqtt_connect_args.username = arg_str0("u", "user", "<user>", "MQTT username");
    mqtt_connect_args.password = arg_str0("p", "pass", "<pass>", "MQTT user password");
    mqtt_connect_args.end = arg_end(4);

    const esp_console_cmd_t mqtt_connect = {
        .command = "mqtt-connect",
        .help = "Connect to an MQTT broker",
        .hint = NULL,
        .func = &mqtt_connect_cmd,
        .argtable = &mqtt_connect_args
    };

    mqtt_subscribe_args.topic = arg_str0(NULL, NULL, "<topic>", "TMQTT topic to subscribe to");
    mqtt_subscribe_args.qos = arg_int0("q", "qos", "<qos>", "MQTT Quality Of Service (QOS) setting");
    mqtt_subscribe_args.qos->ival[0] = 0;
    mqtt_subscribe_args.end = arg_end(2);

    const esp_console_cmd_t mqtt_subscribe = {
        .command = "mqtt-subscribe",
        .help = "Subscribe to data from an MQTT broker",
        .hint = NULL,
        .func = &mqtt_subscribe_cmd,
        .argtable = &mqtt_subscribe_args
    };

    mqtt_publish_args.topic = arg_str0(NULL, NULL, "<topic>", "MQTT topic to publish to");
    mqtt_publish_args.data = arg_str0(NULL, NULL, "<data>", "MQTT data to publish");
    mqtt_publish_args.qos = arg_int0("q", "qos", "<qos>", "MQTT Quality Of Service (QOS) setting");
    mqtt_publish_args.qos->ival[0] = 0;
    mqtt_publish_args.end = arg_end(3);

    const esp_console_cmd_t mqtt_publish = {
        .command = "mqtt-publish",
        .help = "Publish data to an MQTT broker",
        .hint = NULL,
        .func = &mqtt_publish_cmd,
        .argtable = &mqtt_publish_args
    };

     const esp_console_cmd_t mqtt_stop = {
        .command = "mqtt-stop",
        .help = "Stop MQTT broker process",
        .hint = NULL,
        .func = &mqtt_stop_cmd,
        .argtable = NULL
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&mqtt_status) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&mqtt_connect) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&mqtt_subscribe) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&mqtt_publish) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&mqtt_stop) );

}


