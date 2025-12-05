#ifndef LED_H
#define LED_H

#include "driver/gpio.h"
#include "esp_log.h"

#define LED_PIN GPIO_NUM_2


void led_init(void) ;

void led_on(void) ;

void led_off(void) ;



#endif