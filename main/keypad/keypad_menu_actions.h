#ifndef KEYPAD_MENU_ACTIONS_H
#define KEYPAD_MENU_ACTIONS_H

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



void select_option();

void capslock_input();
void enter_service();

void enter_user();







#endif

