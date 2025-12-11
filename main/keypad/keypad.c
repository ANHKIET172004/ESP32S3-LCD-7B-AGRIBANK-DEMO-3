#include "keypad.h"
#include "state_machine/state_machine.h"

//#include "i2c-lcd.h"
#include "esp_mac.h"

#define TAG "KEYPAD"


extern esp_mqtt_client_handle_t mqtt_client;


keypad_context_t g_keypad={
     .input_buffer= {0},
     .buffer_index = 0,
     .wifi_ssid = {0},
     .wifi_pass = {0},
     .wifi_step=0,
     .skip=false,
     .recall=false,
     .pri=false,
     .hide=true,
     .caps_lock = false,
     .last_key = 0,
     .key_press_count = 0,
     .last_key_time = 0,
     .prev_number = {0},
     .saved_input_buffer= {0},
     .saved_buffer_index = 0,
     //.current_mode = MODE_NORMAL,
     .current_mode = MODE_LOGOUT,//
     .saved_ssid = {0},
     .saved_pass = {0},
     .last_t_press_time=0,
     .last_a_press_time=0,
     .last_b_press_time=0,
     .last_s_press_time=0,
     .debounce_interval_us = 1000000,
     .selected_device_id={0},
     .temp_selected_device_name={0},
     .temp_selected_device_id={0},
     .selected_positon=false,
     . positon_flag=0,
     . menu_selection = 0,
     . device_list_ready = false,
     . counter_id={0},
     . default_id={0},
     . device_count=0,
     . service_list_ready = false,
     . service_count=0,
     . selected_service_name={0},
     . selected_service_id={0},
     . switch_device=false,
     . prev_number_status = {0},
     .stop=false,
     .user_pass_buffer={0},
     .user_pass_index=0,
     .selected_option=true,

       
};



extern mutex_context_t g_mutex;





void process_key_wifi_mode(char key) {
    if (key=='A'){
            
        lcd_cursor_right();
            
    }
    
    if (key=='B'){
         
         lcd_send_cmd(0x0C);
         old_screen_reload();
    }
    else if (key == 'D') {
        
        
        enter_wifi();
       
        return;
    }
    else if (key == 'C') {
        delete_wifi_input_key();

        return;
    }
    else if (key == '*') {
        capslock_input();
        return;
    }
    else if (key == '#'&&g_keypad.buffer_index>0) {
        hide_wifi_input();

    }
    /*
    else if (key >= '0' && key <= '9'&&g_keypad.buffer_index<=15) {//
        update_wifi_input_buffer(key);
    }
    */
    else if (key >= '0' && key <= '9'&&g_keypad.buffer_index<=63) {//

        update_wifi_input_buffer(key);
       //ssid_insert_char(get_char(key, NULL));

    }
}





void process_key_normal_mode(char key) {
    if (key >= '0' && key <= '9') {
        update_input_buffer(key);        
        return;
    }

    else if (key == 'D') {
        enter_number();
        return;
    }

    else if (key == 'C') {
        delete_normal_input_key();
        return;
    }

    else if (key == 'A') {
         //uint64_t now = esp_timer_get_time();  
       // if (now - last_a_press_time > debounce_interval_us) {//
           // last_a_press_time = now;

        
        backup_input_buffer();
        
        g_keypad.current_mode = MODE_MENU;
        g_keypad.menu_selection = 1;
        lcd_show_menu();


        return;
        //}//
        //return; 
    }

    else if ((key == 'B')&&(g_keypad.buffer_index<1)) {

        

        call_number();

      
       return; 
    }


    else if ((key == '#')&&(g_keypad.buffer_index<1)) {
        recall_number();
    }

    else if ((key == '*')&&(g_keypad.buffer_index<1)) {

        skip_number();
       
   }//
}

void process_key_device_select(char key) {
    if (key == '1' && g_keypad. selected_index > 0) {
       g_keypad. selected_index--;
        lcd_show_device_list();
    }
    else if (key == '1' && g_keypad. selected_index == 0) {
        g_keypad. selected_index=g_keypad.device_count-1;
        lcd_show_device_list();

    }
    else if (key == '2' && g_keypad.selected_index < g_keypad.device_count - 1) {
        g_keypad.selected_index++;
        lcd_show_device_list();
    }

    else if (key == '2' && g_keypad.selected_index == g_keypad.device_count - 1) {
        g_keypad.selected_index=0;
        lcd_show_device_list();
    }

    else if (key == 'B') {

        old_screen_reload();

    }
    else if (key == 'D') {
       
        publish_device_id();

    }

}

void process_key_user_select(char key) {
    if (key == '1' && g_keypad.selected_index > 0) {
        g_keypad.selected_index--;
        //lcd_show_device_list();
        lcd_show_user_list();
    }

    else if (key == '1' && g_keypad.selected_index == 0) {
        g_keypad.selected_index=g_keypad.device_count - 1;
        //lcd_show_device_list();
        lcd_show_user_list();
    }

    else if (key == '2' && g_keypad.selected_index < g_keypad.device_count - 1) {
        g_keypad.selected_index++;
        //lcd_show_device_list();
        lcd_show_user_list();
    }
    else if  (key == '2' && g_keypad.selected_index == g_keypad.device_count - 1) {
        g_keypad.selected_index=0;
        //lcd_show_device_list();
        lcd_show_user_list();
    }
    else if (key == 'B') {

       old_screen_reload();


    }
    else if ((key == 'D')) {
        enter_user();
    }
    
}


void process_key_service_select(char key) {
    if (key == '1' && g_keypad.selected_index2 > 0) {
        g_keypad.selected_index2--;

        lcd_show_service_list();
    }
    else if (key == '1' && g_keypad.selected_index2 == 0) {
        g_keypad.selected_index2=g_keypad.service_count-1;

        lcd_show_service_list();
    }
    else if (key == '2' && g_keypad.selected_index2 < g_keypad.service_count - 1) {
        g_keypad.selected_index2++;
        lcd_show_service_list();
    }
    else if (key == '2' && g_keypad.selected_index2 == g_keypad.service_count - 1) {
        g_keypad.selected_index2=0;
        lcd_show_service_list();
    }
    else if (key == 'B') {
       set_scroll_enable(false);
       old_screen_reload();

    }
    else if (key == 'D') {
        set_scroll_enable(false);
        enter_service();
    }

}


void process_key_position_select(char key) {
    if (key == '1') {
        g_keypad.selected_positon=false;
        lcd_show_position_list();
    }
    else if (key == '2' ) {
        g_keypad.selected_positon=true;
        lcd_show_position_list();
    }
    else if (key == 'B') {
         g_keypad.current_mode=MODE_SERVICE_SELECT;
         lcd_show_service_list();
    }
    else if (key == 'D') {
        publish_service_id();

    }
   
}


void process_key_menu_mode(char key) {
    if (key == '2') {
        if (g_keypad.menu_selection<8){
        g_keypad.menu_selection++;
        }
        else {
        g_keypad.menu_selection=1;
        }
        lcd_show_menu();
    }
    else if (key == '1') {
        if (g_keypad.menu_selection>1){
            g_keypad.menu_selection--;
        }
        else {
            g_keypad.menu_selection=8;
        }
        lcd_show_menu();
    }
    else if (key == 'B') {
        g_keypad.current_mode = MODE_NORMAL;
        set_display_state(DISPLAY_MAIN_SCREEN);
        g_keypad.menu_selection=0;
        xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);
        lcd_show_main_screen(g_keypad.input_buffer);
        xSemaphoreGive(g_mutex.input_mutex);
    }
    else if (key == 'D') {

         select_option();
       
    }
}

void process_key_logout_mode(char key){
       if (key == 'A') {
        /*
        set_sys_state(STATE_RUNNING);
        g_keypad.current_mode=MODE_NORMAL;

        save_called_number("0000");
        update_temp_buff("0000");

        xSemaphoreTake(g_mutex.mqtt_mutex, portMAX_DELAY);
        esp_mqtt_client_publish(mqtt_client, "reset_number", "reset", 0, 0, 0);
        xSemaphoreGive(g_mutex.mqtt_mutex);
        */

        set_sys_state(STATE_USER_PASS);
        g_keypad.current_mode=MODE_USER_PASS;
        return;
    }
}

void process_key_user_pass(char key){

    if (key>='0'&&key<='9'){
        update_user_pass_buffer(key);
    }

    else if (key == 'B') {
        memset(g_keypad.user_pass_buffer,0,sizeof(g_keypad.user_pass_buffer));
        g_keypad.user_pass_index=0;
        g_keypad.current_mode=MODE_LOGOUT;
        set_sys_state(STATE_LOGOUT);
        //return;
    }

    else if (key == 'C') {
        delete_user_pass_buffer();
        //return;
    }
    else if (key == 'D') {

         check_user_pass();
       
    }
    return ;
}

void process_key_new_user_pass(char key){

    if (key>='0'&&key<='9'){
        update_user_pass_buffer(key);
    }
    

    else if (key=='B'){
       // old_screen_reload();
       memset(g_keypad.user_pass_buffer,0,sizeof(g_keypad.user_pass_buffer));
       g_keypad.user_pass_index=0;
       g_keypad.current_mode=MODE_NORMAL;
       set_sys_state(STATE_RUNNING);

    }
        

    else if (key == 'C') {
        delete_user_pass_buffer();
        //return;
    }
    else if (key == 'D') {

        // check_user_pass();
        save_user_pass(g_keypad.user_pass_buffer);
        memset(g_keypad.user_pass_buffer,0,sizeof(g_keypad.user_pass_buffer));
        g_keypad.user_pass_index=0;
        set_sys_state(STATE_RUNNING);
        g_keypad.current_mode=MODE_NORMAL;
       
    }
    return ;
}


void process_key_option_select(char key) {
    if (key == '1') {
        g_keypad.selected_option=true;
        lcd_show_options();
    }
    else if (key == '2' ) {
        g_keypad.selected_option=false;
        lcd_show_options();
    }
    else if (key == 'B') {

       old_screen_reload();
    }
    else if (key == 'D') {
        if (g_keypad.selected_option==true){

            if (g_keypad.menu_selection==7){

            g_keypad.current_mode=MODE_LOGOUT;//
            set_sys_state(STATE_LOGOUT);//
            }
            else if (g_keypad.menu_selection==6) {

            set_sys_state(STATE_RUNNING);//

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
            }
        }
        else {

            old_screen_reload();
        }

    }
   
}



void process_key(char key) {
    if (g_keypad.current_mode == MODE_NORMAL) {
        process_key_normal_mode(key);
    } else if (g_keypad.current_mode == MODE_WIFI_SSID || g_keypad.current_mode == MODE_WIFI_PASS) {
        process_key_wifi_mode(key);
    }
    else if (g_keypad.current_mode == MODE_MENU) {
        process_key_menu_mode(key);
    }
    else if (g_keypad.current_mode==MODE_DEVICE_SELECT){
        process_key_device_select(key);
    }
     else if (g_keypad.current_mode==MODE_SERVICE_SELECT){
        process_key_service_select(key);
    }

    else if (g_keypad.current_mode==MODE_POSITION_SELECT){
        process_key_position_select(key);
    }
    else if (g_keypad.current_mode==MODE_USER_SELECT){
        process_key_user_select(key);
    }
    else if (g_keypad.current_mode==MODE_LOGOUT){
         process_key_logout_mode(key);
    }
     else if (g_keypad.current_mode==MODE_USER_PASS){
         process_key_user_pass(key);
    }
    else if (g_keypad.current_mode==MODE_NEW_USER_PASS){
        process_key_new_user_pass(key);
    }
    else if (g_keypad.current_mode==MODE_CONTINUE){
        process_key_option_select(key);
    }
}

void keypad_task(void *param) {
    char key;
    char last_pressed_key = 0;
    uint32_t last_press_time = 0;
    esp_task_wdt_add(NULL);  // 
    
    while (1) {
        esp_task_wdt_reset();
        key = keypad_scan();
        if (key != 0) {
            uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
            
            if (key == last_pressed_key && (current_time - last_press_time) < 100) {
                vTaskDelay(pdMS_TO_TICKS(50));
                continue;
            }
            
            last_pressed_key = key;
            last_press_time = current_time;
            
            //ESP_LOGI("KEYPRESED","Key pressed: %c\n", key);
            //printf("Key pressed: %c\n", key);
            process_key(key);
            esp_task_wdt_reset();
            wait_key_release();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        // feed watchdog 
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

