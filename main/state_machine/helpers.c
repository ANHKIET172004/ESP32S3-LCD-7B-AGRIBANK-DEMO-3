#include "state_machine.h"

extern char temp_buff[17];


void wifi_config(){
            esp_err_t err = read_wifi_credentials_from_nvs(g_keypad.saved_ssid, &g_state.ssid_len, g_keypad.saved_pass, &g_state.password_len, NULL);
            if (err == ESP_OK && strlen(g_keypad.saved_ssid) > 0) {
                ESP_LOGI("STATE_MACHINE", "Have a saved wifi network in NVS: %s", g_keypad.saved_ssid);
                
                set_wifi_retry_count(0);
                
                wifi_config_t wifi_config = {0};
                strncpy((char *)wifi_config.sta.ssid, g_keypad.saved_ssid, sizeof(wifi_config.sta.ssid) - 1);
                strncpy((char *)wifi_config.sta.password, g_keypad.saved_pass, sizeof(wifi_config.sta.password) - 1);
                wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
                esp_wifi_disconnect();
                esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
                esp_wifi_stop();
                esp_wifi_start();
                esp_wifi_connect();
                set_sys_state(STATE_WIFI_CONNECT);
            } else {
                lcd_show_message("CHUA LUU WIFI", "");
                vTaskDelay(pdMS_TO_TICKS(800));
                set_sys_state(STATE_RUNNING);
            }
}

void update_temp_buff(const char *display_text) {
    if (display_text && strlen(display_text) > 0) {
        strncpy(temp_buff, display_text, DISPLAY_LINE_MAX);
        temp_buff[DISPLAY_LINE_MAX] = '\0';
    }
}

void reload_oldscreen(){
            esp_task_wdt_reset();

            xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);
            memset(g_keypad.input_buffer, 0, sizeof(g_keypad.input_buffer));
            g_keypad.buffer_index = 0; 
            xSemaphoreGive(g_mutex.input_mutex);   

            size_t num_len = sizeof(g_keypad.prev_number);
            size_t status_len = 12;
            char temp_status[12] = {0};
        
        if (read_current_number_from_nvs(g_keypad.prev_number, &num_len) == ESP_OK && 
            strlen(g_keypad.prev_number) > 0 &&
            read_current_number_status_from_nvs(temp_status, status_len) == ESP_OK &&
            strlen(temp_status) > 0) {
            
            char display[DISPLAY_LINE_MAX + 1] = {0};
            snprintf(display, sizeof(display), "%s:%s", temp_status, g_keypad.prev_number);
            update_temp_buff(display);
        } else {
            update_temp_buff("___");
        }            

            set_sys_state(STATE_MQTT_ERROR);
            set_display_state(DISPLAY_MAIN_SCREEN);

}