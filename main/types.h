#pragma once
#include <stdint.h>

typedef enum {
    STATE_INIT = 0,
    STATE_WIFI_CONNECT,
    STATE_WIFI_SUCCESS,
    STATE_WIFI_ERROR,
    STATE_RUNNING,
    STATE_MQTT_ERROR,
    STATE_LOGOUT,
    STATE_USER_PASS,
    STATE_USER_PASSWORD_ERROR,
    STATE_NEW_USER_PASS,
    STATE_CONTINUE,
    STATE_SAVED_WIFI,
} SystemState;

typedef enum {
    DISPLAY_IDLE = 0,
    DISPLAY_WIFI_CONNECTING = 1,
    DISPLAY_WIFI_SUCCESS = 2,
    DISPLAY_WIFI_ERROR = 3,
    DISPLAY_MQTT_CONNECTING = 4,
    DISPLAY_MAIN_SCREEN = 5,
    DISPLAY_MQTT_ERROR = 6,
    DISPLAY_LOGOUT=7,
    DISPLAY_USER_PASS=8,
    DISPLAY_USER_PASSWORD_ERROR=9,
    DISPLAY_NEW_USER_PASS=10,
    DISPLAY_CONTINUE=11,
    DISPLAY_SAVED_WIFI=12,
} DisplayState;


typedef struct {
      
    char ssid[33];
    char pass[64];
    uint8_t bssid[6];
} wifi_ap_info;

typedef struct {
     uint8_t count;
     wifi_ap_info aps[5];
}  wifi_list;
