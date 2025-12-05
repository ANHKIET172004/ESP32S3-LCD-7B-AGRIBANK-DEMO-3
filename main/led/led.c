#include "led.h"
#define TAG "LED"

void led_init(void) {
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);
    ESP_LOGI(TAG, "LED initialized on GPIO %d", LED_PIN);
}

void led_on(void) {
    gpio_set_level(LED_PIN, 1);
    ESP_LOGI(TAG, "LED ON");
}

void led_off(void) {
    gpio_set_level(LED_PIN, 0);
    ESP_LOGI(TAG, "LED OFF");
}
