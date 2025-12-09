#ifndef HELPERS_H
#define HELPERS_H


#include "stdint.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_mac.h"
#include "lcd_i2c/i2c-lcd.h"
#include "mutex/mutex.h"
#include "keypad/keypad_variables.h"
#include "types.h"




void wifi_config();

void update_temp_buff(const char *display_text) ;

void reload_oldscreen();



#endif