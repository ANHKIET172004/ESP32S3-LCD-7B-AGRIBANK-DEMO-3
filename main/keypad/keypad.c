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
     .current_mode = MODE_NORMAL,
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
        if (g_keypad.menu_selection<7){
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
            g_keypad.menu_selection=7;
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

