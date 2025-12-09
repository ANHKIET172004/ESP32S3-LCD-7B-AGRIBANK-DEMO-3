#ifndef MUTEX_H
#define MUTEX_H


#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "state_machine/state_machine.h"
#include "types.h"

typedef struct{

SemaphoreHandle_t lcd_mutex ;
SemaphoreHandle_t state_mutex ;
SemaphoreHandle_t display_state_mutex ;
SemaphoreHandle_t input_mutex ;
SemaphoreHandle_t device_list_mutex ;
SemaphoreHandle_t service_list_mutex ;
SemaphoreHandle_t wifi_config_mutex ;
SemaphoreHandle_t mqtt_mutex ;
SemaphoreHandle_t selected_id_mutex ;
SemaphoreHandle_t wifi_connected_mutex ;
SemaphoreHandle_t mqtt_connected_mutex ;
SemaphoreHandle_t wifi_start_mutex ;
SemaphoreHandle_t mqtt_start_mutex ;
SemaphoreHandle_t wifi_retry_mutex ;
SemaphoreHandle_t user_selected_wifi_mutex ;
SemaphoreHandle_t scroll_mutex;

} 
mutex_context_t;

extern mutex_context_t g_mutex;



bool get_mqtt_connected(void);
void set_mqtt_connected(bool val);

bool get_wifi_connected(void);
void set_wifi_connected(bool val);

bool get_user_selected_wifi(void);
void set_user_selected_wifi(bool val);

int get_wifi_retry_count(void);
void set_wifi_retry_count(int val);
void increment_wifi_retry_count(void);

SystemState get_sys_state(void);
void set_sys_state(SystemState s);

DisplayState get_display_state(void);
void set_display_state(DisplayState d);

void set_scroll_enable(bool val);
bool get_scroll_enable(void);

void mutex_init(void);

#endif