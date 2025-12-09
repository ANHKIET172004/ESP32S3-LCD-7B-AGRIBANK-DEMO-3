#include "keypad_actions.h"


extern esp_mqtt_client_handle_t mqtt_client;

extern keypad_context_t g_keypad;


void old_screen_reload(){

        strncpy(g_keypad.input_buffer,g_keypad.saved_input_buffer, sizeof(g_keypad.input_buffer) - 1);
        g_keypad.input_buffer[sizeof(g_keypad.input_buffer) - 1]='\0';
        g_keypad.buffer_index = g_keypad.saved_buffer_index;
        
        //g_keypad.current_mode = MODE_NORMAL;
        memset(g_keypad.saved_input_buffer, 0, sizeof(g_keypad.saved_input_buffer));
        g_keypad.saved_buffer_index = 0;
        g_keypad.wifi_step=0;
        g_keypad.last_key = 0;
        g_keypad.key_press_count = 0;
        g_keypad.caps_lock = false;

        
        g_keypad.current_mode=MODE_MENU;
        lcd_show_menu();
}


void backup_input_buffer(){
        xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);
        strncpy(g_keypad.saved_input_buffer, g_keypad.input_buffer, sizeof(g_keypad.saved_input_buffer) - 1);
        g_keypad.saved_input_buffer[sizeof(g_keypad.saved_input_buffer) - 1]='\0';
        g_keypad.saved_buffer_index = g_keypad.buffer_index;
        xSemaphoreGive(g_mutex.input_mutex);
}


void delete_normal_input_key(){
        xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);
        if (g_keypad.buffer_index > 0) {
            g_keypad.buffer_index--;
            g_keypad.input_buffer[g_keypad.buffer_index] = '\0';
            g_keypad.last_key = 0;
            g_keypad.key_press_count = 0;
            set_display_state(DISPLAY_MAIN_SCREEN);
            if (g_keypad.buffer_index > 0) {
                update_temp_buff(g_keypad.input_buffer);
            } else {
                
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
            }
        }
        xSemaphoreGive(g_mutex.input_mutex);

}






void update_input_buffer(char key){
    xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);
    if (g_keypad.buffer_index >= 4) {
        
        g_keypad.buffer_index = 0;
    }
        g_keypad.input_buffer[g_keypad.buffer_index] = key;
        g_keypad.buffer_index++;
        g_keypad.input_buffer[g_keypad.buffer_index] = '\0';
    //set_display_state(DISPLAY_MAIN_SCREEN);
    update_temp_buff(g_keypad.input_buffer);
    xSemaphoreGive(g_mutex.input_mutex);

}

void enter_number(){
     if (g_keypad.buffer_index > 3) {
            g_keypad.pri = true;
            xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);
            mqtt_publish_number(g_keypad.input_buffer);
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
            update_temp_buff("___");//
        }

        }

}


void call_number(){
       esp_task_wdt_reset();//

        uint64_t now = esp_timer_get_time();  
        if (now -g_keypad.last_b_press_time > g_keypad.debounce_interval_us) {//
            g_keypad.last_b_press_time = now;

        g_keypad.pri = false;
        if (!get_mqtt_connected()) { 
           
           reload_oldscreen();
        } else if(get_mqtt_connected())  {
            
            g_keypad.skip=false;
            //set_publish(true); 
             char json_msg[64];
            //const char *json_msg = "{\"device_id\":\"04:1A:2B:3C:4D:04\",\"request\":\"number\"}";
            //sprintf(json_msg, "{\"device_id\":\"%s\",\"request\":\"number\"}",selected_id);
            sprintf(json_msg, "{\"device_id\":\"%s\",\"request\":\"number\"}",g_keypad.selected_device_id);//
            xSemaphoreTake(g_mutex.mqtt_mutex, portMAX_DELAY);
            esp_mqtt_client_publish(mqtt_client, "requestnumber", json_msg, 0, 0, 0);
            xSemaphoreGive(g_mutex.mqtt_mutex);

        }
    
        return;
       }//

}





void skip_number(){
     g_keypad.skip=true;
        uint64_t now = esp_timer_get_time();  
        if (now -g_keypad.last_s_press_time > g_keypad.debounce_interval_us) {//
            g_keypad.last_s_press_time = now;
        g_keypad.pri = false;
        if (!get_mqtt_connected()) { 

           reload_oldscreen();
        } else if (get_mqtt_connected()) {
            
            xSemaphoreTake(g_mutex.mqtt_mutex, portMAX_DELAY);
            //esp_mqtt_client_publish(mqtt_client, "skipnumber_display", "skip", 0, 0, 0);
            char json_msg[128];
            sprintf(json_msg,"{\"device_id\":\"%s\"}",g_keypad.selected_device_id);//
            esp_mqtt_client_publish(mqtt_client, "skipnumber", json_msg, 0, 0, 0);
            xSemaphoreGive(g_mutex.mqtt_mutex);

            
        }
        return;
    }

};

void recall_number(){
     g_keypad.recall=true;
        uint64_t now = esp_timer_get_time();  
        if (now - g_keypad.last_t_press_time > g_keypad.debounce_interval_us) {//
            g_keypad.last_t_press_time = now;
         if (!get_mqtt_connected()) { 
            reload_oldscreen();

        } else if(get_mqtt_connected()) {
            char json_msg[128];
            sprintf(json_msg,"{\"device_id\":\"%s\"}",g_keypad.selected_device_id);//
            xSemaphoreTake(g_mutex.mqtt_mutex, portMAX_DELAY);
            esp_mqtt_client_publish(mqtt_client, "recallnumber", json_msg, 0, 0, 0);
            xSemaphoreGive(g_mutex.mqtt_mutex);

        }
        return;
    }//

};

void publish_device_id(){

        g_keypad.skip=true;

        backup_input_buffer();


        xSemaphoreTake(g_mutex.device_list_mutex, portMAX_DELAY);//
        strncpy(g_keypad.temp_selected_device_id, g_keypad.device_list[g_keypad.selected_index].device_id, sizeof(g_keypad.temp_selected_device_id) - 1);//
        g_keypad.temp_selected_device_id[sizeof(g_keypad.temp_selected_device_id) - 1]='\0';
        xSemaphoreGive(g_mutex.device_list_mutex);//

        mqtt_publish_device_id(g_keypad.temp_selected_device_id);//
        esp_mqtt_client_publish(mqtt_client, "transfer_number", "YES", 0, 0, true);//


        g_keypad.in_selection_mode = false;
        g_keypad.current_mode = MODE_NORMAL;
        set_display_state(DISPLAY_MAIN_SCREEN);

};

void publish_service_id(){
            if (g_keypad. selected_positon==false){
            g_keypad.positon_flag=false;
        }

        else {
           g_keypad.positon_flag=true;
        }
        g_keypad.skip=true;
        mqtt_publish_service_id(g_keypad.selected_service_id);//
        esp_mqtt_client_publish(mqtt_client, "transfer_number", "YES", 0, 0, true);//

       
        //in_selection_mode = false;
        g_keypad.current_mode = MODE_NORMAL;
        set_display_state(DISPLAY_MAIN_SCREEN);
};

