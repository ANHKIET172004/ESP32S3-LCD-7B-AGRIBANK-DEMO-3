#ifndef ESP_MQTT_ACTIONS_H
#define ESP_MQTT_ACTIONS_H


#include "esp_mqtt_client.h"


void mqtt_publish_number(const char *number) ;

void mqtt_publish_device_id(const char *id) ;

void mqtt_publish_service_id(const char *id);



#endif