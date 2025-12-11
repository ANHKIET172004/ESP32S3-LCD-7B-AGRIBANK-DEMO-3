#ifndef KEYPAD_H
#define KEYPAD_H


#include "stdint.h"
#include "stdbool.h"
#include <stddef.h>
#include "esp_mqtt_client/esp_mqtt_client.h"
#include "nvs_utils/nvs_utils.h"
#include "keypad_driver.h"
#include "T9.h"

#include "keypad_actions.h"
#include "keypad_variables.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <time.h>
#include "nvs_flash.h"
#include "lcd_i2c/i2c-lcd.h"
#include "lcd_i2c/lcd_option_handler.h"
#include "mutex/mutex.h"
#include "keypad_wifi_actions.h"
#include "keypad_menu_actions.h"
#include "keypad_cursor_actions.h"


enum Mode {
    MODE_NORMAL = 0,
    MODE_WIFI_SSID = 1,
    MODE_WIFI_PASS = 2,
    MODE_MENU=3,
    MODE_DEVICE_SELECT = 4,
    MODE_SERVICE_SELECT = 5,
    MODE_POSITION_SELECT=6,
    MODE_USER_SELECT=7,
    MODE_LOGOUT=8,
    MODE_USER_PASS=9,
    MODE_NEW_USER_PASS=10,
    MODE_CONTINUE=11,

};


typedef struct {
    const char *accent;
    const char *replacement;
} accent_map_t;


extern uint8_t start_cnt ;



void process_key_wifi_mode(char key);
void process_key_normal_mode(char key);
void process_key_device_select(char key);
void process_key_user_select(char key) ;
void process_key_service_select(char key);

void process_key_position_select(char key);
void process_key_menu_mode(char key) ;
void process_key_new_user_pass(char key);
void process_key_option_select(char key) ;
void process_key(char key) ;
void old_screen_reload();

void keypad_task(void *param) ;






#endif