#ifndef ESP_MQTT_CLIENT_H
#define ESP_MQTT_CLIENT_H

#include "stdint.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "keypad/keypad.h"
#include "cJSON.h"
#include "lcd_i2c/i2c-lcd.h"
#include "led/led.h"
#include "nvs_utils/nvs_utils.h"
#include "mutex/mutex.h"
#include "esp_mqtt_actions.h"
#include "esp_mqtt_topic_handler.h"
#include "mqtt_types.h"

#define MQTT_QUEUE_LENGTH 19


void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void mqtt_init(void) ;


void mqtt_process_task(void *pvParameters) ;

void custom_string(const char *input, char *output, size_t output_len) ;

int extract_counter_number(const char *name) ;



#endif