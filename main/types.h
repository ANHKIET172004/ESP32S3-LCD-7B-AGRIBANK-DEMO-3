#pragma once
#include <stdint.h>

typedef enum {
    STATE_INIT = 0,
    STATE_WIFI_CONNECT,
    STATE_WIFI_SUCCESS,
    STATE_WIFI_ERROR,
    STATE_RUNNING,
    STATE_MQTT_ERROR,
} SystemState;

typedef enum {
    DISPLAY_IDLE = 0,
    DISPLAY_WIFI_CONNECTING = 1,
    DISPLAY_WIFI_SUCCESS = 2,
    DISPLAY_WIFI_ERROR = 3,
    DISPLAY_MQTT_CONNECTING = 4,
    DISPLAY_MAIN_SCREEN = 5,
    DISPLAY_MQTT_ERROR = 6,
} DisplayState;
