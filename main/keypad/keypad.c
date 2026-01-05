#include "keypad.h"
#include "state_machine/state_machine.h"

//#include "i2c-lcd.h"
#include "esp_mac.h"

#define TAG "KEYPAD"


extern esp_mqtt_client_handle_t mqtt_client;

extern char counter_id[3];

extern uint8_t scroll_cnt;

extern bool requestskip;
extern uint8_t service_scroll_pos;

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
     //.current_mode = MODE_LOGOUT,//
     .saved_ssid = {0},
     .saved_pass = {0},
     .saved_bssid={0},
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
     .wifi_position=0,
     .delete_wifi_option=0,
     .view={0},

       
};

extern mutex_context_t g_mutex;

void process_key_wifi_mode(char key) {
    

    if (key >= '0' && key <= '9'&&g_keypad.buffer_index<=63) {//

        update_wifi_input_buffer(key);
       return;

    }
    switch (key){
    case 'A':         
        lcd_cursor_right();         
        break;
    
    case 'B':       
         lcd_send_cmd(0x0C);
         old_screen_reload();
         break; 
    case 'D':       
        enter_wifi(); 
        //return;
        break;
    case 'C':
        delete_wifi_input_key();

        //return;
        break;

    case '*': 
        capslock_input();
        //return;
        break;
    
    case'#': 
        if (g_keypad.buffer_index>0) {
        hide_wifi_input();

         }
         break;

    default:
       break;
  }
}





void process_key_normal_mode(char key) {
    if (key >= '0' && key <= '9') {
        update_input_buffer(key);        
        return;
    }

    switch (key){
        case 'D':
            enter_number();
            break;
        

        case 'C':
            delete_normal_input_key();
            break;
        

        case 'A':
            //uint64_t now = esp_timer_get_time();  
        // if (now - last_a_press_time > debounce_interval_us) {//
            // last_a_press_time = now;

            
            backup_input_buffer();   
            g_keypad.current_mode = MODE_MENU;
            g_keypad.menu_selection = 1;
            set_sys_state(STATE_MENU);//
            break;

        case 'B':
            if (g_keypad.buffer_index<1) {
                
                call_number();
            
            }
                break;


        case '#':
            if (g_keypad.buffer_index<1) {
                recall_number();
            }
                break;

        case '*':
            if (g_keypad.buffer_index<1) {

                skip_number();
            
        }//
        break;

    default:
        break;

    }
}

void process_key_device_select(char key) {
    switch (key){
    case '1':
     
         if (g_keypad. selected_index > 0) {
            g_keypad. selected_index--;
        
         }
         else {

            g_keypad. selected_index=g_keypad.device_count-1;
            
         }
         if (strcmp(g_keypad.device_list[g_keypad.selected_index].device_id, g_keypad.default_id)==0){//
            if (g_keypad. selected_index > 0) {
              g_keypad. selected_index--;
        
            }
            else {

              g_keypad. selected_index=g_keypad.device_count-1;
            
         }
            }
    break;

    case'2' :

        if (g_keypad.selected_index < g_keypad.device_count - 1) {
        g_keypad.selected_index++;
        
       } 
       else {
          g_keypad.selected_index=0;
       }
        if (strcmp(g_keypad.device_list[g_keypad.selected_index].device_id, g_keypad.default_id)==0){//
            if (g_keypad.selected_index < g_keypad.device_count - 1) {
                g_keypad.selected_index++;
        
            } 
            else {
                g_keypad.selected_index=0;
            }
            }
    break;

    case 'B':

        old_screen_reload();

        break;
    case 'D':      
        publish_device_id();
        break;

    default:
       break;
  }

}

void process_key_user_select(char key) {

    switch (key){
    case '1': 
        if (g_keypad.selected_index > 0){
        g_keypad.selected_index--;
        }
        else {
         g_keypad.selected_index=g_keypad.device_count - 1;
        }
        break;


    case  '2': 
       if (g_keypad.selected_index < g_keypad.device_count - 1){
        g_keypad.selected_index++;
       }
       else {
        g_keypad.selected_index=0;
       }
        break;
  
    case 'B':
       old_screen_reload();
       break; 

    case'D':
        enter_user();
        break;

    default:
        break;
   }
    
}


void process_key_service_select(char key) {
    switch (key){

    case '1':
     lcd_clear();//
     service_scroll_pos=0;
     if (g_keypad.selected_index2 > 0) {
        g_keypad.selected_index2--;

        
    } else {
        g_keypad.selected_index2=g_keypad.service_count-1;
    }

    break;
    

    case '2':
    lcd_clear();//
    service_scroll_pos=0;//
     if ( g_keypad.selected_index2 < g_keypad.service_count - 1) {
        g_keypad.selected_index2++;
        
    } else {
        g_keypad.selected_index2=0;
    }

    break;

    
   
    case 'B':
       set_scroll_enable(false);
       scroll_cnt=0;//
       old_screen_reload();

       break;
    case 'D':
        set_scroll_enable(false);
        scroll_cnt=0;//
        g_keypad.selected_positon=false;//
        enter_service();
        break;
    
    default:
        break;
          

  }

}


void process_key_position_select(char key) {
    switch (key){
    case '1':
        //set_scroll_enable(false);//
        //lcd_clear();//
        g_keypad.selected_positon=false;
        //lcd_show_position_list();
        
        break;

    case '2':
       // set_scroll_enable(false);//
        //lcd_clear();//
        g_keypad.selected_positon=true;
        //lcd_show_position_list();     
        break;

    case 'B':
         //lcd_clear();//
         g_keypad.current_mode=MODE_SERVICE_SELECT;
         set_sys_state(STATE_SERVICE_LIST);//
         //lcd_show_service_list();     
         break;

    case 'D':
        
        publish_service_id();

         break;

    default:
        break;
   }
}


void process_key_menu_mode(char key) {
    switch (key)
    {
    case '2':
        if (g_keypad.menu_selection<7){
            g_keypad.menu_selection++;
        }
        else {
            g_keypad.menu_selection=1;
        }
        break;
    case '1':
        if (g_keypad.menu_selection>1){
            g_keypad.menu_selection--;
        }
        else {
            g_keypad.menu_selection=7;
        }
        break;
    case 'B':
        g_keypad.current_mode = MODE_NORMAL;
        set_sys_state(STATE_RUNNING);//

        break;
    case 'D':
         select_option();
        break;
    
    default:
        break;
    }
    
}

void process_key_logout_mode(char key){
       if (key == 'A') {
        
        set_sys_state(STATE_RUNNING);
        g_keypad.current_mode=MODE_NORMAL;

        save_called_number("");
        update_temp_buff("");

        //xSemaphoreTake(g_mutex.mqtt_mutex, portMAX_DELAY);
        esp_mqtt_client_publish(mqtt_client, "reset_number", "reset", 0, 0, 0);
        //xSemaphoreGive(g_mutex.mqtt_mutex);
        
/*
        set_sys_state(STATE_USER_PASS);
        g_keypad.current_mode=MODE_USER_PASS;
        */
        return;
    }
}



void process_key_option_select(char key) {
    switch(key){
    
        case '1':
            g_keypad.selected_option=true;
            break;
        
        case '2':
            g_keypad.selected_option=false;
            break;

        case 'B':

            old_screen_reload();
            break;

        case 'D':
            if (g_keypad.selected_option==true){
                if (!get_mqtt_connected()) {     
                reload_oldscreen();
                return;
                }

                if (g_keypad.menu_selection==6){
                    esp_mqtt_client_publish(mqtt_client, "closed", "closed", 0, 0, 0);// gửi mess đóng qầy để màn hình display
                    g_keypad.current_mode=MODE_LOGOUT;//
                    set_sys_state(STATE_LOGOUT);//
                    }
                else if (g_keypad.menu_selection==7) {
                    set_sys_state(STATE_RUNNING);//
                    //char saved_counter_id[4] = {0};

                      save_called_number("");//

                    uint8_t mac[6];
                    char mac_str[18];

                    if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK) {
                        snprintf(mac_str, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
                                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                        ESP_LOGI("DEMO", "WiFi STA MAC: %s", mac_str);
                    }

                    strcpy(g_keypad.default_id,mac_str);
                    
                    esp_mqtt_client_publish(mqtt_client, "reset_number", "reset", 0, 0, 0);

                    //
                    counter_id_init();// mac
                    strcpy(counter_id,g_keypad.counter_id);
                    strcpy(g_keypad.selected_device_id,g_keypad.default_id);
                    //
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

        
            break;

        default:
        break;
    }  


}



void process_key(char key)
{
    switch (g_keypad.current_mode)
    {
        case MODE_NORMAL:
            process_key_normal_mode(key);
            break;

        case MODE_WIFI_SSID:
        case MODE_WIFI_PASS:
            process_key_wifi_mode(key);
            break;

        case MODE_MENU:
            process_key_menu_mode(key);
            break;

        case MODE_DEVICE_SELECT:
            process_key_device_select(key);
            break;

        case MODE_SERVICE_SELECT:
            process_key_service_select(key);
            break;

        case MODE_POSITION_SELECT:
            process_key_position_select(key);
            break;

        case MODE_USER_SELECT:
            process_key_user_select(key);
            break;

        case MODE_LOGOUT:
            process_key_logout_mode(key);
            break;

        case MODE_CONTINUE:
            process_key_option_select(key);
            break;

        default:
            break;
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

