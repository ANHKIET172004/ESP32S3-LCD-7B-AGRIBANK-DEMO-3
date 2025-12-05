#ifndef WIFI_H
#define WIFI_H

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "lwip/inet.h"
#include "lwip/ip4_addr.h"
#include "mutex/mutex.h"
#include "keypad/keypad_variables.h"




#define WIFI_MAX_RETRY 5
#define WIFI_ERROR_TIMEOUT_MS 1000
#define WIFI_SUCCESS_TIMEOUT_MS 1000


void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) ;

void wifi_init(void) ;

#endif