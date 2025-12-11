#ifndef LCD_OPTION_HANDLER_H
#define LCD_OPTION_HANDLER_H


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi/wifi.h"
#include "keypad/keypad.h"
#include "state_machine/state_machine.h"
#include "mutex/mutex.h"
#include "driver/i2c.h"
#include "nvs_utils/nvs_utils.h"



void lcd_show_main_screen(const char *number) ;
void lcd_show_wifi_input(const char *ssid) ;
void lcd_show_wifi_pass(const char *pass) ;
void lcd_show_user_pass(const char* number);

void lcd_show_message(const char *line1, const char *line2);

void lcd_show_device_list(void) ;

void lcd_show_user_list(void) ;


void lcd_show_position_list(void) ;

void lcd_show_service_list(void);

void lcd_show_menu(void) ;

void lcd_mainscreen_init();

void lcd_show_new_user_pass(const char* number);

void lcd_show_options(void) ;



#endif