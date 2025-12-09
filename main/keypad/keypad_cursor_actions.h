#ifndef KEYPAD_CURSOR_ACTIONS_H
#define KEYPAD_CURSOR_ACTIONS_H

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
#include "mutex/mutex.h"
#include "keypad_wifi_actions.h"
#include "keypad_menu_actions.h"



void lcd_render_ssid_editor() ;

void ssid_insert_char(char c);


void ssid_delete() ;


void lcd_cursor_right() ;

void ssid_delete_at_cursor() ;






#endif