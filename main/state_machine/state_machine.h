#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

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
#include "helpers.h"

#define DISPLAY_UPDATE_INTERVAL 50
#define STATE_DISPLAY_DURATION 1000


typedef struct {


DisplayState display_state ;
DisplayState prev_display_state;
uint32_t display_update_time;

SystemState sys_state;
SystemState prev_sys_state ;
uint32_t state_enter_time ;
TaskHandle_t system_task_handle;
char selected_ssid[17];
char selected_pass[17] ;
size_t ssid_len ;
size_t password_len;


} state_context_t;

extern state_context_t g_state;

void handle_display(void) ;

//void handle_system_state_machine(void);


void update_display_state(void) ;


//void reload_oldscreen();

void system_state_update();


void system_task(void *pvParameters);

#endif