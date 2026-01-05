#include "wifi.h"
#include "esp_mqtt_client/esp_mqtt_client.h"
#include "keypad/keypad.h"
#include "state_machine/state_machine.h"

#define TAG "WIFI"

#define MAX_AP 20


extern keypad_context_t g_keypad;


wifi_ap_record_t ap;

extern bool start;


void wifi_scan()
{
    uint16_t num = MAX_AP;

    wifi_ap_record_t *ap_info = calloc(MAX_AP, sizeof(wifi_ap_record_t));
    wifi_list *list = calloc(1, sizeof(wifi_list));

    if (!ap_info || !list) {
        ESP_LOGE(TAG, "malloc failed");
        free(ap_info);
        free(list);
        return;
    }

    int best_rssi    = -1000;
    int best_saved_index = -1;


    
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true
    };

    load_wifi_list(list);  

    if (list->count==0){
        set_sys_state(STATE_NO_WIFI);
        return;
    }

    int tmp_rssi[list->count];
    int tmp_cnt[list->count];

    for (int i=0;i<list->count;i++){
           tmp_rssi[i]=0;
           tmp_cnt[i]=0;
    }


    ESP_LOGI(TAG,"%d saved wifi in nvs",list->count);

    for (uint8_t i=0;i<list->count;i++){
            
         ESP_LOGI(TAG,"SAVED WIFI: %s",list->aps[i].ssid);
            
    }

    for (int a=0;a<3;a++){

////
    num = MAX_AP;
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&num, ap_info));

    ESP_LOGI(TAG, "Found %d APs", num);


    for (uint8_t i = 0; i < num; i++) {

        for (uint8_t j = 0; j < list->count; j++) {

            if ((strcmp((char*)ap_info[i].ssid, list->aps[j].ssid) == 0)&&(memcmp(ap_info[i].bssid,list->aps[j].bssid,6))==0) {

                ESP_LOGI(TAG, "FOUND SAVED WIFI: %s (RSSI=%d), BSSID: %02X:%02X:%02X %02X:%02X:%02X",ap_info[i].ssid,ap_info[i].rssi,
        ap_info[i].bssid[0],ap_info[i].bssid[1],ap_info[i].bssid[2],
        ap_info[i].bssid[3],ap_info[i].bssid[4],ap_info[i].bssid[5]);
             
                tmp_rssi[j]+=ap_info[i].rssi;
                tmp_cnt[j]++;
                
            }
        }
    }

    

}   
for (int i=0;i<list->count;i++){
        if (tmp_cnt[i]==0) continue;;
        if ((tmp_rssi[i]/tmp_cnt[i]) > best_rssi) {
                    best_rssi = tmp_rssi[i]/tmp_cnt[i];
                    best_saved_index = i;
                }
    }

    if (best_saved_index < 0) {
        ESP_LOGW(TAG, "No saved WiFi found in scan");
        free(ap_info);
        free(list);
        set_sys_state(STATE_NO_WIFI);//
        return;
    }
      ESP_LOGI(TAG, "BEST WIFI: %s (RSSI=%d)",list->aps[best_saved_index].ssid,best_rssi);//

    strcpy(g_keypad.saved_ssid, list->aps[best_saved_index].ssid);
    strcpy(g_keypad.saved_pass, list->aps[best_saved_index].pass);
    memcpy(g_keypad.saved_bssid,list->aps[best_saved_index].bssid,6);
    g_keypad.best_saved_index=best_saved_index;//

    memset(g_keypad.connecting_wifi, 0, sizeof(g_keypad.connecting_wifi));
    strncpy(g_keypad.connecting_wifi,g_keypad.saved_ssid,sizeof(g_keypad.connecting_wifi)-1);//
    g_keypad.connecting_wifi[sizeof(g_keypad.connecting_wifi)-1]='\0';//

    free(ap_info);
    free(list);
}


 void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi STA started");
        //esp_wifi_connect();
       // wifi_scan();//
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "WiFi connected, got IP");
        set_sys_state(STATE_WIFI_SUCCESS);
        g_keypad.current_mode = MODE_NORMAL;//
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
          
           // save_wifi_credentials1(g_keypad.wifi_ssid, g_keypad.wifi_pass, ap.bssid);
           // memset(g_keypad.connecting_wifi, 0, sizeof(g_keypad.connecting_wifi));
           // strncpy(g_keypad.connecting_wifi,g_keypad.wifi_ssid,sizeof(g_keypad.connecting_wifi)-1);
           // g_keypad.connecting_wifi[sizeof(g_keypad.connecting_wifi)-1]='\0';
            



        }

    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        wifi_event_sta_connected_t *ev = (wifi_event_sta_connected_t *) event_data;
        ESP_LOGI(TAG, "Connected to SSID:%s, channel:%d", ev->ssid, ev->channel);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* ev = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGI(TAG, "STA disconnected, reason=%d", ev->reason);
       // g_keypad.connecting_wifi[0]='\0';//
       //memset(g_keypad.connecting_wifi, 0, sizeof(g_keypad.connecting_wifi));
       //sprintf(g_keypad.connecting_wifi,"KHONG CO WIFI");


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
        else if (!get_user_selected_wifi()){// lỗi sau khi wifi đã kết nối hoặc khi reconnect lại wifi lúc khởi động
         
            increment_wifi_retry_count();  
            int retry_count = get_wifi_retry_count();  
            if (retry_count==1){
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

