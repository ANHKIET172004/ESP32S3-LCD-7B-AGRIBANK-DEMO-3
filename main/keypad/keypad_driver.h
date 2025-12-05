#ifndef KEYPAD_DRIVER_H
#define KEYPAD_DRIVER_H

#include "driver/gpio.h"
#include "esp_task_wdt.h"


#define ROW1 GPIO_NUM_13
#define ROW2 GPIO_NUM_12
#define ROW3 GPIO_NUM_14
#define ROW4 GPIO_NUM_27

#define COL1 GPIO_NUM_26
#define COL2 GPIO_NUM_25
#define COL3 GPIO_NUM_33
#define COL4 GPIO_NUM_32



void keypad_init() ;

char keypad_scan();

void wait_key_release() ;


#endif