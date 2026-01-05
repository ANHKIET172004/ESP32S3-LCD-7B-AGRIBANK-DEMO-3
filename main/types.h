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
    STATE_CONTINUE,
    STATE_MENU,
    STATE_DEVICE_LIST,
    STATE_SERVICE_LIST,
    STATE_SERVICE_POSITON,
    STATE_WIFI_INPUT,
    STATE_NO_DATA,
    STATE_NO_WIFI,
    STATE_WIFI_RETRY,
} SystemState;

typedef enum {
    DISPLAY_IDLE = 0,
    DISPLAY_WIFI_CONNECTING = 1,
    DISPLAY_WIFI_SUCCESS = 2,
    DISPLAY_WIFI_ERROR = 3,
    DISPLAY_MAIN_SCREEN = 4,
    DISPLAY_MQTT_ERROR = 5,
    DISPLAY_LOGOUT=6,
    DISPLAY_CONTINUE=7,
    DISPLAY_MENU=8,
    DISPLAY_DEVICE_LIST=9,
    DISPLAY_SERVICE_LIST=10,
    DISPLAY_SERVICE_POSITION=11,
    DISPLAY_WIFI_INPUT=12,
    DISPLAY_NO_DATA=13,
    DISPLAY_NO_WIFI=14,
    DISPLAY_WIFI_RETRY=15,


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
