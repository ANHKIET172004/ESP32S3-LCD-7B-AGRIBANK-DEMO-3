#include "wifi.h"
#include "esp_mqtt_client/esp_mqtt_client.h"
#include "keypad/keypad.h"
#include "state_machine/state_machine.h"
#include "init_handle/init_handle.h"

#define TAG "WIFI"

#define MAX_AP 20


extern keypad_context_t g_keypad;


wifi_ap_record_t ap;

extern bool start;


void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi STA started");
        //esp_wifi_connect();
       // wifi_scan();//
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "WiFi connected, got IP");
        set_sys_state(STATE_WIFI_SUCCESS);
        //g_keypad.current_mode = MODE_NORMAL;//
        g_keypad.current_mode=MODE_NO_KEY;//
        set_wifi_retry_count(0);

        set_wifi_connected(true);

        if (esp_wifi_sta_get_ap_info(&ap) == ESP_OK) {
            ESP_LOGI(TAG,
                "Connected BSSID: %02X:%02X:%02X:%02X:%02X:%02X",
                ap.bssid[0], ap.bssid[1], ap.bssid[2],
                ap.bssid[3], ap.bssid[4], ap.bssid[5]);

            ESP_LOGI(TAG, "SSID: %s, RSSI: %d", ap.ssid, ap.rssi);
        }



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
                //g_keypad.current_mode = MODE_NORMAL;//
                set_user_selected_wifi(false);
                g_keypad.current_mode=MODE_NO_KEY;//
                set_sys_state(STATE_WIFI_ERROR);
                return;
            } 
            else if ((retry_count < WIFI_MAX_RETRY)&&(retry_count >0)){
            ESP_LOGW(TAG, "WiFi connection failed, retry count: %d/%d", retry_count, WIFI_MAX_RETRY);
            esp_wifi_connect();
            }
          
        } 
        else if (!get_user_selected_wifi()){// lỗi sau khi wifi đã kết nối hoặc khi reconnect lại wifi lúc khởi động
         
            increment_wifi_retry_count();  
            int retry_count = get_wifi_retry_count();  
            if (retry_count==1){// kết nối thất bại sẽ chuyển sang state retry
                g_keypad.current_mode=MODE_NO_KEY;//
                set_sys_state(STATE_WIFI_RETRY);
            }
            if (retry_count < WIFI_MAX_RETRY) {
                esp_wifi_connect();
            } else {
                ESP_LOGE(TAG, "Saved WiFi connection failed after %d retries", WIFI_MAX_RETRY);
                set_sys_state(STATE_WIFI_ERROR);
            }
        }
    }
}

/*
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
    */

void wifi_init(void)
{
    esp_err_t ret;

    // Tạo netif cho STA
    esp_netif_create_default_wifi_sta();

    // Init WiFi với config mặc định
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(ret));
        init_fail_handle(2);// restart nếu số lần restart còn trong mức cho phép
        //return;           
    }
    if (read_retry(2)>0){// khởi tạo thành công-> reset số lần đã restart trước đó (nếu có)
        reset_retry(2);
    }

    ret = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Register WIFI_EVENT failed: %s", esp_err_to_name(ret));
        init_fail_handle(3);
        //return;
    }
    if (read_retry(3)>0){
        reset_retry(3);
    }

    ret = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Register IP_EVENT failed: %s", esp_err_to_name(ret));
        init_fail_handle(4);
        //return;
    }
    if (read_retry(4)>0){
        reset_retry(4);
    }

    wifi_config_t wifi_config = {0};  

    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_mode failed: %s", esp_err_to_name(ret));
        init_fail_handle(5);

        //return;
    }
    if (read_retry(5)>0){// khởi tạo thành công-> reset số lần đã restart trước đó (nếu có)
        reset_retry(5);
    }

    ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_config failed: %s", esp_err_to_name(ret));
        init_fail_handle(6);

        //return;
    }
    if (read_retry(6)>0){// khởi tạo thành công-> reset số lần đã restart trước đó (nếu có)
        reset_retry(6);
    }

    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_start failed: %s", esp_err_to_name(ret));
        init_fail_handle(7);

        //return;
    }
    if (read_retry(7)>0){// khởi tạo thành công-> reset số lần đã restart trước đó (nếu có)
        reset_retry(7);
    }

    ESP_LOGI(TAG, "WiFi initialized successfully");
}


