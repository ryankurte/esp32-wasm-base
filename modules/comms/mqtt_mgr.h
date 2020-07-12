
#ifndef MQTT_MGR_H
#define MQTT_MGR_H

// Initialise the MQTT manager
int MQTT_MGR_init();

// Publish data using the MQTT connection
int MQTT_MGR_publish(char* topic, char* data, int qos);

// Subscribe to incoming MQTT data
int MQTT_MGR_subscribe(char* topic, int qos);

// Register MQTT CLI commands
void MQTT_MGR_register_commands();


#endif
