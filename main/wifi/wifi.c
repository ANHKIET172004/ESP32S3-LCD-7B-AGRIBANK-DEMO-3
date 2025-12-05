#include "wifi.h"
#include "esp_mqtt_client/esp_mqtt_client.h"
#include "keypad/keypad.h"
#include "state_machine/state_machine.h"

#define TAG "WIFI"


extern keypad_context_t g_keypad;



 void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi STA started");
        //esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "WiFi connected, got IP");
        set_sys_state(STATE_WIFI_SUCCESS);
        g_keypad.current_mode = MODE_NORMAL;//
        //set_mqtt_start(true);
        set_wifi_retry_count(0);

        set_wifi_connected(true);



        if (get_user_selected_wifi()) {  
            set_user_selected_wifi(false);  
            save_wifi_credentials(g_keypad.wifi_ssid, g_keypad.wifi_pass, NULL);


        }
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        wifi_event_sta_connected_t *ev = (wifi_event_sta_connected_t *) event_data;
        ESP_LOGI(TAG, "Connected to SSID:%s, channel:%d", ev->ssid, ev->channel);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* ev = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGI(TAG, "STA disconnected, reason=%d", ev->reason);



        if (get_user_selected_wifi()) { //trường hợp lỗi khi người dùng nhập wifi trực tiếp từ keypad
            increment_wifi_retry_count();  
            int retry_count = get_wifi_retry_count(); 
            
            
            if (retry_count == WIFI_MAX_RETRY) {
                ESP_LOGE(TAG, "WiFi connection failed after %d retries", WIFI_MAX_RETRY);
                set_wifi_retry_count(0);//
                g_keypad.current_mode = MODE_NORMAL;//
                set_user_selected_wifi(false);
                set_sys_state(STATE_WIFI_ERROR);
                return;
            } 
            else if ((retry_count < WIFI_MAX_RETRY)&&(retry_count >0)){
            ESP_LOGW(TAG, "WiFi connection failed, retry count: %d/%d", retry_count, WIFI_MAX_RETRY);
            esp_wifi_connect();
            }
          
        } 
        else{// lỗi sau khi wifi đã kết nối
         
            increment_wifi_retry_count();  
            int retry_count = get_wifi_retry_count();  
            if (retry_count < WIFI_MAX_RETRY) {
                esp_wifi_connect();
            } else {
                ESP_LOGE(TAG, "Saved WiFi connection failed after %d retries", WIFI_MAX_RETRY);
                set_sys_state(STATE_WIFI_ERROR);
            }
        }
    }
}


void wifi_init(void) {
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {0};
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "WiFi initialized");
}

