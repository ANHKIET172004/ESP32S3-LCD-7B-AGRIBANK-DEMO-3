#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_task_wdt.h"
#include "lcd_i2c/i2c-lcd.h"
#include "state_machine/state_machine.h"
#include "esp_mqtt_client/esp_mqtt_client.h"
#include "keypad/keypad.h"
#include "wifi/wifi.h"
#include "nvs_utils/nvs_utils.h"
#include "led/led.h"
#include "mac_utils/mac_utils.h"

#define TAG "DEMO"

uint8_t start_cnt = 0;
bool user_selected_wifi = false;

char display_user_id[3]={0};


extern QueueHandle_t mqtt_queue;

extern state_context_t g_state;

extern void service_scroll_task(void *pvParameter);

void app_main(void) {

    mutex_init();
    led_init();
    led_off();  
    keypad_init();
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");
    lcd_mainscreen_init();
    ESP_LOGI(TAG, "Starting keypad MQTT application");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    user_id_init();// mac
    strcpy(display_user_id,g_keypad.user_id);
    strcpy(g_keypad.selected_device_id,g_keypad.default_id);

    esp_netif_init();
    esp_event_loop_create_default();
    wifi_init();

    mqtt_queue = xQueueCreate(MQTT_QUEUE_LENGTH, sizeof(mqtt_message_t));
    if (mqtt_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create MQTT queue!");
    } 
    //xTaskCreate(mqtt_process_task, "mqtt_task", 6*1024, NULL, 4, NULL);

    xTaskCreatePinnedToCore(mqtt_process_task, "mqtt_task", 6* 1024, NULL, 4, NULL,0 );
    xTaskCreatePinnedToCore(keypad_task, "keypad_task", 7* 1024, NULL, 6, NULL, 1);
    xTaskCreatePinnedToCore( system_task, "system_task",  4*1024, NULL,5, &g_state.system_task_handle, 1 );
    xTaskCreate(service_scroll_task, "service_scroll_task", 2048, NULL, 1, NULL);



}