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
#include "esp_heap_caps.h"

#define TAG "DEMO"

uint8_t start_cnt = 0;
bool start=true;
bool user_selected_wifi = false;

char counter_id[3]={0};

bool start1=true;



extern QueueHandle_t mqtt_queue;

extern state_context_t g_state;

extern void service_scroll_task(void *pvParameter);

extern void ssid_scroll_task(void *pvParameter) ;


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

    counter_id_init();// mac
    strcpy(counter_id,g_keypad.counter_id);
    strcpy(g_keypad.selected_device_id,g_keypad.default_id);

    esp_netif_init();
    esp_event_loop_create_default();
    wifi_init();
    size_t ssid_len=sizeof(g_keypad.saved_ssid);
    size_t pass_len=sizeof(g_keypad.saved_pass);
   // if (read_wifi_credentials_from_nvs(g_keypad.saved_ssid,g_keypad.saved_pass,g_keypad.saved_bssid)!=ESP_OK){
    //esp_err_t err = read_wifi_credentials_from_nvs(g_keypad.saved_ssid, &g_state.ssid_len, 
      //                 g_keypad.saved_pass, &g_state.password_len, NULL);
    esp_err_t err = read_wifi_credentials_from_nvs(g_keypad.saved_ssid, &ssid_len, 
    g_keypad.saved_pass, &pass_len, NULL);
    /*
    if (err != ESP_OK) {
        wifi_scan();
    }
    else {
        wifi_list *list = calloc(1, sizeof(wifi_list));
        

        if (!list) {
            ESP_LOGE(TAG, "malloc failed");
            free(list);
        }
        else {
            load_wifi_list(list);
            for (int i=0;i<list->count;i++){
                if (strncmp(g_keypad.saved_ssid,list->aps[i].ssid,sizeof(g_keypad.saved_ssid)-1)==0){
                    g_keypad.best_saved_index=i;
                    memset(g_keypad.connecting_wifi, 0, sizeof(g_keypad.connecting_wifi));
                   strncpy(g_keypad.connecting_wifi,g_keypad.saved_ssid,sizeof(g_keypad.connecting_wifi)-1);//
                   g_keypad.connecting_wifi[sizeof(g_keypad.connecting_wifi)-1]='\0';//
                }
            }
            free(list);

        }
    }
        */

    mqtt_queue = xQueueCreate(MQTT_QUEUE_LENGTH, sizeof(mqtt_message_t));
    if (mqtt_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create MQTT queue!");
    } 
    save_wifi_credentials1("NGUYEN HUYNH ANH KIET 2002","1234",NULL);
    xTaskCreatePinnedToCore(mqtt_process_task, "mqtt_task", 6* 1024, NULL, 4, NULL,0 );
    xTaskCreatePinnedToCore(keypad_task, "keypad_task", 7* 1024, NULL, 6, NULL, 1);
    xTaskCreatePinnedToCore( system_task, "system_task",  4*1024, NULL,5, &g_state.system_task_handle, 1 );
    xTaskCreate(service_scroll_task, "service_scroll_task", 2048, NULL, 4, NULL);
    xTaskCreate(ssid_scroll_task, "ssid_scroll_task", 2048, NULL, 4, NULL);

    while (1){
        size_t free_heap = esp_get_free_heap_size();
       printf("Free heap: %d bytes\n", free_heap);
       vTaskDelay(pdMS_TO_TICKS(1000));
    }
}