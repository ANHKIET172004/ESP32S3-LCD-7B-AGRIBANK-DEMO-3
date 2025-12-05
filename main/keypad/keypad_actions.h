#ifndef KEYPAD_ACTIONS_H
#define KEYPAD_ACTIONS_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "keypad.h"

#define WIFI_MAX_LEN 63
#define LCD_LEN 16

extern SemaphoreHandle_t input_mutex;


void old_screen_reload();


void backup_input_buffer();
void delete_normal_input_key();
//void delete_wifi_input_key();


//void hide_wifi_input();

void update_input_buffer(char key);

//void update_wifi_input_buffer(char key);


void enter_number();

//void enter_wifi();


void call_number();

void skip_number();

void recall_number();

void publish_device_id();

void publish_service_id();




#endif