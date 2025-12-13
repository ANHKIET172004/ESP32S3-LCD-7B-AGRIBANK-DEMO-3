#include "wifi.h"
#include "esp_mqtt_client/esp_mqtt_client.h"
#include "keypad/keypad.h"
#include "state_machine/state_machine.h"

#define TAG "WIFI"

#define MAX_AP 20



extern keypad_context_t g_keypad;

static bool ascii_ssid(const char* ssid){
      int cnt=0;
         while (*(ssid+cnt)!='\0'){
             if (*(ssid+cnt)>127){
                return false;
             }
             cnt++;
         }
         return true;
}

void wifi_scan1(){
    
    uint16_t count;
    uint16_t num= MAX_AP;
    wifi_ap_record_t ap_info[MAX_AP];

    wifi_list list;

    int8_t best_rssi=-127;// khoi tao

    wifi_scan_config_t scan_config = {
    .ssid = 0,
    .bssid = 0,
    .channel = 0,
    .show_hidden = true
  };


    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config,true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&num,ap_info));
    //ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&count));
    ESP_LOGI(TAG,"Found %d ap",num);

    load_wifi_list(&list);

    for (uint8_t i=0;i<num;i++){
        for (uint8_t j=0;j<2;j++){
            if (strcmp((char*)ap_info[i].ssid,list.aps[j].ssid)==0){
                 ESP_LOGI(TAG,"FOUND SAVED WIFI, %s",ap_info[i].ssid);
            }
          }

    }

    esp_wifi_scan_stop();
}

void wifi_scan2()
{
    uint16_t num = MAX_AP;
    wifi_ap_record_t ap_info[MAX_AP];
    wifi_list list;

    int8_t best_rssi    = -127;
    int best_ap_index   = -1;
    int best_saved_index = -1;

    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true
    };

    // Scan blocking
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&num, ap_info));

    ESP_LOGI(TAG, "Found %d APs", num);

    // Load saved WiFi list
    load_wifi_list(&list);

    for (uint8_t i = 0; i < num; i++) {

        // bỏ qua SSID có UTF-8
        //if (!ascii_ssid((char*)ap_info[i].ssid))
          //  continue;

        for (uint8_t j = 0; j < list.count; j++) {

            if (strcmp((char*)ap_info[i].ssid, list.aps[j].ssid) == 0) {

                ESP_LOGI(TAG, "FOUND SAVED WIFI: %s (RSSI=%d)",
                         ap_info[i].ssid, ap_info[i].rssi);

                
                if (ap_info[i].rssi > best_rssi) {
                    best_rssi = ap_info[i].rssi;
                    best_ap_index = i;
                    best_saved_index = j;
                }
            }
        }
    }

    // Không tìm thấy WiFi phù hợp
    if (best_ap_index < 0) {
        ESP_LOGW(TAG, "No saved WiFi found in scan");
        return;
    }

    ESP_LOGI(TAG, "FOUND SAVED WIFI: %s (RSSI=%d), BSSID: %s",
                         ap_info[best_ap_index].ssid, ap_info[best_ap_index].rssi,ap_info[best_ap_index].bssid);

    //strcpy(g_keypad.saved_ssid, list.aps[best_saved_index].ssid);
    //strcpy(g_keypad.saved_pass, list.aps[best_saved_index].pass);

    // Chuẩn bị kết nối
    /*
    wifi_config_t cfg = {0};

    strcpy((char*)cfg.sta.ssid,     list.aps[best_saved_index].ssid);
    strcpy((char*)cfg.sta.password, list.aps[best_saved_index].pass);

    ESP_LOGI(TAG, "Connecting to strongest saved WiFi: %s (RSSI=%d)",
             cfg.sta.ssid, best_rssi);

    esp_wifi_set_config(WIFI_IF_STA, &cfg);
    esp_wifi_connect();
    */
}


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

    int8_t best_rssi    = -127;
    int best_ap_index   = -1;
    int best_saved_index = -1;

    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true
    };

    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&num, ap_info));

    ESP_LOGI(TAG, "Found %d APs", num);

    load_wifi_list(list);  

    for (uint8_t i = 0; i < num; i++) {

        for (uint8_t j = 0; j < list->count; j++) {

            if (strcmp((char*)ap_info[i].ssid, list->aps[j].ssid) == 0) {

                ESP_LOGI(TAG, "FOUND SAVED WIFI: %s (RSSI=%d), BSSID: %02X:%02X:%02X %02X:%02X:%02X",ap_info[i].ssid,ap_info[i].rssi,
        ap_info[i].bssid[0],ap_info[i].bssid[1],ap_info[i].bssid[2],
        ap_info[i].bssid[3],ap_info[i].bssid[4],ap_info[i].bssid[5]);

                if (ap_info[i].rssi > best_rssi) {
                    best_rssi = ap_info[i].rssi;
                    best_ap_index = i;
                    best_saved_index = j;
                }
            }
        }
    }

    if (best_ap_index < 0) {
        ESP_LOGW(TAG, "No saved WiFi found in scan");
        free(ap_info);
        free(list);
        return;
    }

    ESP_LOGI(TAG, "BEST WIFI: %s (RSSI=%d), BSSID: %02X:%02X:%02X %02X:%02X:%02X",ap_info[best_ap_index].ssid,ap_info[best_ap_index].rssi,
        ap_info[best_ap_index].bssid[0],ap_info[best_ap_index].bssid[1],ap_info[best_ap_index].bssid[2],
        ap_info[best_ap_index].bssid[3],ap_info[best_ap_index].bssid[4],ap_info[best_ap_index].bssid[5]);

    strcpy(g_keypad.saved_ssid, list->aps[best_saved_index].ssid);
    strcpy(g_keypad.saved_pass, list->aps[best_saved_index].pass);
    //strncpy((char*)g_keypad.saved_bssid,(char*)ap_info[best_ap_index].bssid,sizeof((char*)g_keypad.saved_bssid) );// dùng strncpy, dùng strcpy treo code
    memcpy(g_keypad.saved_bssid,list->aps[best_ap_index].bssid,6);

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
        //set_mqtt_start(true);
        set_wifi_retry_count(0);

        set_wifi_connected(true);



        if (get_user_selected_wifi()) {  
            set_user_selected_wifi(false);  
            //save_wifi_credentials(g_keypad.wifi_ssid, g_keypad.wifi_pass, NULL);
            save_wifi_credentials1(g_keypad.wifi_ssid, g_keypad.wifi_pass, NULL);



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

