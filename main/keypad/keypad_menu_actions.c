
#include "keypad_menu_actions.h"

extern char counter_id[3];

extern esp_mqtt_client_handle_t mqtt_client;


void select_option(){
      backup_input_buffer();

        
        if (g_keypad.menu_selection == 1) {

            g_keypad.ssid_real_pos=0;
            g_keypad.wifi_len=0;

            
            xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);
            //strncpy(saved_input_buffer, input_buffer, sizeof(saved_input_buffer) - 1);
            //saved_buffer_index = buffer_index;

            g_keypad.current_mode = MODE_WIFI_SSID;
            memset(g_keypad.input_buffer, 0, sizeof(g_keypad.input_buffer));
            g_keypad.buffer_index = 0;
            xSemaphoreGive(g_mutex.input_mutex);
            g_keypad.wifi_step = 0;
            g_keypad.last_key = 0;
            g_keypad.key_press_count = 0;
            g_keypad.caps_lock = false;
            g_keypad.hide = true;
            lcd_show_wifi_input("");
            lcd_put_cur(1,0);//
            lcd_send_cmd(0x0F);//
        }

        else if (g_keypad.menu_selection == 2) {
           if (g_keypad.device_list_ready && g_keypad.device_count > 0) {
                g_keypad.selected_index = 0;
                g_keypad.in_selection_mode = true;
                g_keypad.current_mode = MODE_DEVICE_SELECT;  
                lcd_show_device_list();
            } else {
                lcd_show_message("CHUA CO DU LIEU", "THU LAI SAU");
                vTaskDelay(pdMS_TO_TICKS(800));
                g_keypad.current_mode = MODE_NORMAL;
            }
        }

        else if (g_keypad.menu_selection == 3) {
           if (g_keypad.service_list_ready && g_keypad.service_count > 0) {
                g_keypad.selected_index2 = 0;
                g_keypad.in_selection_mode = true;
                g_keypad.current_mode = MODE_SERVICE_SELECT;  
                lcd_show_service_list();
            } else {
                lcd_show_message("CHUA CO DU LIEU", "THU LAI SAU");
                vTaskDelay(pdMS_TO_TICKS(800));
                g_keypad.current_mode = MODE_NORMAL;
            }
        }

        else if (g_keypad.menu_selection == 4) {
               char json_msg[128]={0};
               sprintf(json_msg,"Device_id:");
               uint8_t mac[6];
              char mac_str[128];

            if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK) {
                snprintf(mac_str, 128,"{\"device_id\":\"%02X:%02X:%02X:%02X:%02X:%02X\"}",
                        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                ESP_LOGI("DEMO", "Registered device with WiFi STA MAC: %s", mac_str);
            }

               //strcpy(selected_id,mac_str);
               xSemaphoreTake(g_mutex.mqtt_mutex, portMAX_DELAY);

               esp_mqtt_client_publish(mqtt_client, "device/register", mac_str , 0, 0, 0);
               xSemaphoreGive(g_mutex.mqtt_mutex);


               g_keypad.current_mode=MODE_NORMAL;
               
        }

         else if (g_keypad.menu_selection == 5) {
           if (g_keypad.device_list_ready && g_keypad.device_count > 0) {
                g_keypad.selected_index = 0;
                g_keypad.in_selection_mode = true;
                g_keypad.current_mode = MODE_USER_SELECT;  
                //lcd_show_device_list();
                lcd_show_user_list();
            } else {
                lcd_show_message("CHUA CO DU LIEU", "THU LAI SAU");
                vTaskDelay(pdMS_TO_TICKS(1500));
                g_keypad.current_mode = MODE_NORMAL;
            }
        }



        else if (g_keypad.menu_selection==6){
               
            g_keypad.selected_option=true;//
            g_keypad.current_mode=MODE_CONTINUE;
            set_sys_state(STATE_CONTINUE);
             
            /*
            
            char saved_counter_id[4] = {0};
            esp_err_t err = read_counter_id_from_nvs(saved_counter_id, sizeof(saved_counter_id));

            if (err == ESP_OK && strlen(saved_counter_id) > 0) {
                // Có counter_id đã lưu
                strncpy(g_keypad.counter_id, saved_counter_id, sizeof(g_keypad.counter_id) - 1);
                g_keypad.counter_id[sizeof(g_keypad.counter_id) - 1] = '\0';
                ESP_LOGI("DEMO", "Loaded counter_id from NVS: %s", g_keypad.counter_id);
            } else {
                // Không có counter_id, dùng mặc định 00
                strcpy(g_keypad.counter_id, "00");
                ESP_LOGI("DEMO", "No saved g_keypad.counter_id, using default: %s", g_keypad.counter_id);
            }

            
            save_called_number("0000");


            uint8_t mac[6];
            char mac_str[18];

            if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK) {
                snprintf(mac_str, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
                        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                ESP_LOGI("DEMO", "WiFi STA MAC: %s", mac_str);
            }

            strcpy(g_keypad.default_id,mac_str);
            
            xSemaphoreTake(g_mutex.mqtt_mutex, portMAX_DELAY);
            esp_mqtt_client_publish(mqtt_client, "reset_number", "reset", 0, 0, 0);
            xSemaphoreGive(g_mutex.mqtt_mutex);


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
            update_temp_buff("___");//
        }   
            //save_login_status("NO");
            g_keypad.current_mode = MODE_NORMAL;
            */

        }

        else if (g_keypad.menu_selection==7){
            g_keypad.selected_option=true;//
            g_keypad.current_mode=MODE_CONTINUE;
            set_sys_state(STATE_CONTINUE);
            /*
            xSemaphoreTake(g_mutex.mqtt_mutex, portMAX_DELAY);
            esp_mqtt_client_publish(mqtt_client, "closed", "closed", 0, 0, 0);
            xSemaphoreGive(g_mutex.mqtt_mutex);
            g_keypad.stop=true;
            //g_keypad.current_mode = MODE_NORMAL;
            g_keypad.current_mode=MODE_LOGOUT;//
            set_sys_state(STATE_LOGOUT);//
            */

        }
        /*
        else if (g_keypad.menu_selection==8){
            xSemaphoreTake(g_mutex.mqtt_mutex, portMAX_DELAY);
            esp_mqtt_client_publish(mqtt_client, "open", "open", 0, 0, 0);
            xSemaphoreGive(g_mutex.mqtt_mutex);
            g_keypad.stop=false;
            g_keypad.current_mode = MODE_NORMAL;
            

        }
            */
        else if (g_keypad.menu_selection==8){

            set_sys_state(STATE_NEW_USER_PASS);
            g_keypad.current_mode = MODE_NEW_USER_PASS;
            

        }
        else if (g_keypad.menu_selection==9){
            set_sys_state(STATE_SAVED_WIFI);
            g_keypad.current_mode=MODE_SAVED_WIFI;
        }


}


void capslock_input() {
    g_keypad.caps_lock = !g_keypad.caps_lock;

    xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);

    char view[17];

    if (g_keypad.wifi_len <= 16) {
        memcpy(view, g_keypad.input_buffer, g_keypad.wifi_len);
        view[g_keypad.wifi_len] = '\0';
    } else {
        memcpy(view,
               &g_keypad.input_buffer[g_keypad.ssid_window_start],
               16);
        view[16] = '\0';
    }

    if (g_keypad.wifi_step == 0){
        lcd_show_wifi_input(view);
    }

    else{
        lcd_show_wifi_pass(view);
    }

    // CURSOR
    
    int cursor_col = g_keypad.ssid_real_pos - g_keypad.ssid_window_start;
    if (cursor_col < 0) cursor_col = 0;
    if (cursor_col > 15) cursor_col = 15;

    lcd_put_cur(1, cursor_col);
    lcd_send_cmd(0x0F);

    xSemaphoreGive(g_mutex.input_mutex);


}



void enter_service(){
            if (g_keypad.selected_index2>-1){


        backup_input_buffer();

        
        xSemaphoreTake(g_mutex.service_list_mutex, portMAX_DELAY);//
        strncpy(g_keypad.selected_service_name, g_keypad.service_list[g_keypad.selected_index2].name, sizeof(g_keypad.selected_service_name) - 1);
        g_keypad.selected_service_name[sizeof(g_keypad.selected_service_name) - 1]='\0';

        strncpy(g_keypad.selected_service_id, g_keypad.service_list[g_keypad.selected_index2].device_id, sizeof(g_keypad.selected_service_id) - 1);
        g_keypad.selected_service_id[sizeof(g_keypad.selected_service_id) - 1]='\0';
        //strncpy(device_id, selected_device_id, sizeof(device_id) - 1);
        xSemaphoreGive(g_mutex.service_list_mutex);//

        //mqtt_publish_service_id(selected_service_id);
        // strcpy(selected_id,selected_service_id);
         
         //ESP_LOGI("DEMO","%s",selected_id);
        lcd_show_position_list();
        }

       // in_selection_mode = false;
        //current_mode = MODE_NORMAL;
        //display_state = DISPLAY_MAIN_SCREEN;
        g_keypad.current_mode=MODE_POSITION_SELECT;
}

void enter_user(){
        g_keypad.switch_device=true;

        backup_input_buffer();

        xSemaphoreTake(g_mutex.device_list_mutex, portMAX_DELAY);//

        if (g_keypad.switch_device==true){
        strncpy(g_keypad.selected_device_name, g_keypad.device_list[g_keypad.selected_index].name, sizeof(g_keypad.selected_device_name) - 1);
        g_keypad.selected_device_name[sizeof(g_keypad.selected_device_name) - 1]='\0';
        strncpy(g_keypad.selected_device_id, g_keypad.device_list[g_keypad.selected_index].device_id, sizeof(g_keypad.selected_device_id) - 1);
        g_keypad.selected_device_id[sizeof(g_keypad.selected_device_id) - 1]='\0';
        char recent_counter_id[4]={0};
        snprintf(recent_counter_id, sizeof(recent_counter_id), "%02d", g_keypad.selected_index+1);
        snprintf(recent_counter_id, sizeof(recent_counter_id), "%s", g_keypad.device_list[g_keypad.selected_index].counter_id);
        strncpy(g_keypad.counter_id,recent_counter_id,sizeof(g_keypad.counter_id)-1);
        g_keypad.counter_id[sizeof(g_keypad.counter_id)-1]='\0';
        strncpy(counter_id,g_keypad.counter_id,sizeof(counter_id)-1);
        counter_id[sizeof(counter_id)-1]='\0';
        g_keypad.switch_device=false;
        }
        
        xSemaphoreGive(g_mutex.device_list_mutex);//

        g_keypad.in_selection_mode = false;
        g_keypad.current_mode = MODE_NORMAL;
        set_display_state(DISPLAY_MAIN_SCREEN);
};
