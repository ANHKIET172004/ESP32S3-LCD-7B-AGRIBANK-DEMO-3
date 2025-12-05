#ifndef ESP_MQTT_TOPIC_HANDLER_H
#define ESP_MQTT_TOPIC_HANDLER_H

#include "esp_mqtt_client.h"
#include "mqtt_types.h"
#include "mac_utils/mac_utils.h"

 void responsenumber_topic_handler(mqtt_message_t msg);
 void device_list_handler(mqtt_message_t msg);

 void service_list_handler(mqtt_message_t msg);



#endif