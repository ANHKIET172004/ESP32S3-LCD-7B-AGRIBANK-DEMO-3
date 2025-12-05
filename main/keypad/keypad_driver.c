#include "keypad_driver.h"

const char keypad[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

 gpio_num_t rows[4] = {ROW1, ROW2, ROW3, ROW4};
 gpio_num_t cols[4] = {COL1, COL2, COL3, COL4};

 void keypad_init() {
    for (int i = 0; i < 4; i++) {
        gpio_reset_pin(rows[i]);
        gpio_set_direction(rows[i], GPIO_MODE_OUTPUT);
        gpio_set_level(rows[i], 1);
    }
    for (int i = 0; i < 4; i++) {
        gpio_reset_pin(cols[i]);
        gpio_set_direction(cols[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(cols[i], GPIO_PULLUP_ONLY);
    }
}

char keypad_scan() {
    for (int row = 0; row < 4; row++) {
        for (int i = 0; i < 4; i++) {
            gpio_set_level(rows[i], (i == row) ? 0 : 1);
        }
        vTaskDelay(pdMS_TO_TICKS(5));
        for (int col = 0; col < 4; col++) {
            if (gpio_get_level(cols[col]) == 0) {
                vTaskDelay(pdMS_TO_TICKS(20));
                if (gpio_get_level(cols[col]) == 0) {
                    return keypad[row][col];
                }
            }
        }
    }
    for (int i = 0; i < 4; i++) gpio_set_level(rows[i], 1);
    return 0;
}
void wait_key_release() {
    uint32_t timeout = xTaskGetTickCount() + pdMS_TO_TICKS(3000);  // Max 3 giây

    while (xTaskGetTickCount() < timeout) {
        esp_task_wdt_reset();  // Vẫn phải feed WDT trong lúc chờ

        bool key_still_pressed = false;
        for (int col = 0; col < 4; col++) {
            if (gpio_get_level(cols[col]) == 0) {
                key_still_pressed = true;
                break;
            }
        }
        if (!key_still_pressed) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}





