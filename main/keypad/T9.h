#ifndef T9_H
#define T9_H


#include "stdint.h"
#include "stdbool.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "keypad_variables.h"

#define T9_TIMEOUT_MS 700



char get_char(char key, int *should_replace);





#endif
