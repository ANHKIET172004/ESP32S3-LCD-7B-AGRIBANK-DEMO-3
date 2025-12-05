#include "esp_mqtt_actions.h"

static char* TAG= "MQTT";
extern esp_mqtt_client_handle_t mqtt_client;

void mqtt_publish_number(const char *number) {
    if (!get_mqtt_connected()) {
        
        reload_oldscreen();
        return;
    }
    char json_msg[128] = {0};

    snprintf(json_msg, sizeof(json_msg), "{\"device_id\":\"%s\",\"number\":\"%s\"}",g_keypad.selected_device_id, number);//
    xSemaphoreTake(g_mutex.mqtt_mutex, portMAX_DELAY);

    esp_mqtt_client_publish(mqtt_client, "specificnumber", json_msg, 0, 0, 0);
    xSemaphoreGive(g_mutex.mqtt_mutex);

    ESP_LOGI(TAG, "Published number %s", number);
}



void mqtt_publish_device_id(const char *id) {
    if (!get_mqtt_connected()){
        reload_oldscreen();
        return;
    }
        size_t num_len = sizeof(g_keypad.prev_number);
        
        
        if (read_current_number_from_nvs(g_keypad.prev_number, &num_len) == ESP_OK && 
        strlen(g_keypad.prev_number) > 0){
        xSemaphoreTake(g_mutex.mqtt_mutex, portMAX_DELAY);

        esp_mqtt_client_publish(mqtt_client, "skipnumber_display", "skip", 0, 0, 0);//
        char json_msg[128] = {0};
        snprintf(json_msg, sizeof(json_msg), "{\"number\":\"%s\",\"device_id\":\"%s\"}", g_keypad.prev_number,id);
        esp_mqtt_client_publish(mqtt_client, "transfernumber", json_msg, 0, 0, 0);
        xSemaphoreGive(g_mutex.mqtt_mutex);

        ESP_LOGI(TAG, "Published id %s", id);
            }
}


void mqtt_publish_service_id(const char *id) {
    if (!get_mqtt_connected()){
        reload_oldscreen();
        return;
       
    } 

        size_t num_len = sizeof(g_keypad.prev_number);
       
        
        if (read_current_number_from_nvs(g_keypad.prev_number, &num_len) == ESP_OK && 
        strlen(g_keypad.prev_number) > 0){
        char json_msg[128] = {0};
        xSemaphoreTake(g_mutex.mqtt_mutex, portMAX_DELAY);

        esp_mqtt_client_publish(mqtt_client, "skipnumber_display", "skip", 0, 0, 0);//
        if (g_keypad.selected_positon==false){
        snprintf(json_msg, sizeof(json_msg), "{\"number\":\"%s\",\"service_id\":\"%s\",\"position\":1}", g_keypad.prev_number,id);
        }
        else {
        snprintf(json_msg, sizeof(json_msg), "{\"number\":\"%s\",\"service_id\":\"%s\",\"position\":0}", g_keypad.prev_number,id);

        }
        esp_mqtt_client_publish(mqtt_client, "transferservice", json_msg, 0, 0, 0);
        xSemaphoreGive(g_mutex.mqtt_mutex);

        ESP_LOGI(TAG, "Published id %s", id);
            }
}