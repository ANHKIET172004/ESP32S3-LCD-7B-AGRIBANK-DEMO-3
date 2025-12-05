#ifndef MQTT_TYPES_H
#define MQTT_TYPES_H


#define MQTT_MAX_TOPIC_LEN 64
#define MQTT_MAX_PAYLOAD_LEN 1024

typedef struct {
    char topic[MQTT_MAX_TOPIC_LEN];
    char payload[MQTT_MAX_PAYLOAD_LEN];
} mqtt_message_t;







#endif