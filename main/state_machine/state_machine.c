#include "state_machine.h"
#include "wifi/wifi.h"

#define TAG "STATE_MACHINE"
 char temp_buff[17] = {0};

state_context_t g_state={

    . display_state = DISPLAY_IDLE,
    . prev_display_state = DISPLAY_IDLE,
    . display_update_time = 0,
    //. sys_state = STATE_INIT,
    //. prev_sys_state = STATE_INIT,
    . sys_state = STATE_LOGOUT,//
    . prev_sys_state = STATE_LOGOUT,//
    . state_enter_time = 0,
    . system_task_handle = NULL,
    . selected_ssid = {0},
    . selected_pass = {0},
    . ssid_len = sizeof(g_state.selected_ssid),
    . password_len = sizeof(g_state.selected_pass),

};



void handle_display(void) {
    uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    if (get_display_state()==g_state.prev_display_state){
        if (current_time - g_state.display_update_time < DISPLAY_UPDATE_INTERVAL) {
            return;
        }
    }
    
    
    switch (get_display_state()){
        case DISPLAY_WIFI_CONNECTING:
            lcd_show_message("TRANG THAI WIFI:", "DANG KET NOI...");
            break;
        
        case DISPLAY_WIFI_SUCCESS:
            
            xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);
            strncpy(g_keypad.input_buffer, g_keypad.saved_input_buffer, sizeof(g_keypad.input_buffer) - 1);//
            g_keypad.buffer_index = g_keypad.saved_buffer_index;//
            xSemaphoreGive(g_mutex.input_mutex);
            lcd_show_message("TRANG THAI WIFI:", "THANH CONG!");
           
            break;
        
        case DISPLAY_WIFI_ERROR:
            lcd_show_message("TRANG THAI WIFI:", "THAT BAI!");
            
            break;
        case DISPLAY_MQTT_ERROR:
            lcd_show_message("GOI THAT BAI!", "KHONG KET NOI!");
           
            break;
        case DISPLAY_LOGOUT:
            lcd_show_message("  DA DANG XUAT!","  \"A\"- >LOG IN");
            break;
        case DISPLAY_USER_PASS:

            if (g_keypad.user_pass_index>0){
            lcd_show_user_pass(g_keypad.user_pass_buffer);
            }
            else {
             lcd_show_user_pass("____");   
            }
            
            break;
        case DISPLAY_NEW_USER_PASS:

             if (g_keypad.user_pass_index>0){
            lcd_show_new_user_pass(g_keypad.user_pass_buffer);
            }
            else {
             lcd_show_new_user_pass("____");   
            }
            
        
        case DISPLAY_MAIN_SCREEN:
            if (g_keypad.current_mode == MODE_NORMAL) {
                if (g_keypad.buffer_index > 0) {
                    xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);
                    lcd_show_main_screen(g_keypad.input_buffer);
                    xSemaphoreGive(g_mutex.input_mutex);
                } else {
                    lcd_show_main_screen(temp_buff);
                }
            }
            break;

        case DISPLAY_USER_PASSWORD_ERROR:
             lcd_clear();
             lcd_put_cur(0,0);
             lcd_send_string("SAI USER PASS!");
             lcd_put_cur(1,0);
             lcd_send_string("THU LAI SAU!");
             break;
        case DISPLAY_CONTINUE:
            lcd_show_options();
            break;
        case DISPLAY_IDLE:
        default:
            break;
    }
    
    g_state.prev_display_state = get_display_state();
}


void update_display_state(void) {
    
    switch (get_sys_state()) {
        case STATE_INIT:
            set_display_state(g_state.display_state);
            break;
        
        case STATE_WIFI_CONNECT:
            set_display_state(DISPLAY_WIFI_CONNECTING);

            break;
        
        case STATE_WIFI_SUCCESS:
            set_display_state(DISPLAY_WIFI_SUCCESS);

            break;
        
        case STATE_WIFI_ERROR:
            set_display_state(DISPLAY_WIFI_ERROR);

            break;

        case STATE_MQTT_ERROR:
            set_display_state(DISPLAY_MQTT_ERROR);

            break;
        
        case STATE_RUNNING:
            set_display_state(DISPLAY_MAIN_SCREEN);
            break;
        case STATE_LOGOUT:
            set_display_state(DISPLAY_LOGOUT);
            break;
        case STATE_USER_PASS:
             set_display_state(DISPLAY_USER_PASS);
             break;
        case STATE_USER_PASSWORD_ERROR:
             set_display_state(DISPLAY_USER_PASSWORD_ERROR);
             break;

        case STATE_NEW_USER_PASS:
             set_display_state(DISPLAY_NEW_USER_PASS);
             break;
        case STATE_CONTINUE:
             set_display_state(DISPLAY_CONTINUE);
             break;
        
        default:
            set_display_state(DISPLAY_IDLE);
            break;
    }
}



void system_state_update(){

        
        switch (get_sys_state()) {
            case STATE_INIT:
            {
                wifi_config();
                break;
            }

            case STATE_WIFI_CONNECT:
              
                 
                break;

            case STATE_WIFI_ERROR:
            {
                uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
                if (get_sys_state() !=g_state. prev_sys_state) {
                    g_state.state_enter_time = current_time;
                }
                
                if (current_time - g_state.state_enter_time >= STATE_DISPLAY_DURATION) {
                    set_sys_state(STATE_RUNNING);
                }
                esp_task_wdt_reset();
                break;
            }

            case STATE_WIFI_SUCCESS:
            {
                uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
                if (get_sys_state() != g_state.prev_sys_state) {
                    g_state.state_enter_time = current_time;
                }
                
                if (current_time - g_state.state_enter_time >= STATE_DISPLAY_DURATION) {
                    set_sys_state(STATE_RUNNING);//
                    mqtt_init();
                }
                esp_task_wdt_reset();
                break;
            }

             case STATE_MQTT_ERROR:
            {
                uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
                if (get_sys_state() != g_state.prev_sys_state) {
                    g_state.state_enter_time = current_time;
                }
                
                if (current_time - g_state.state_enter_time >= STATE_DISPLAY_DURATION) {
                    set_sys_state(STATE_RUNNING);
                }
                esp_task_wdt_reset();
                break;
            }


            case STATE_RUNNING:
                break;
            
            case STATE_LOGOUT:
                 break;
            case STATE_USER_PASS:
                 break;
            case STATE_NEW_USER_PASS:
                 break;
            case STATE_CONTINUE:
                 break;
            case STATE_USER_PASSWORD_ERROR:
            uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
                if (get_sys_state() !=g_state. prev_sys_state) {
                    g_state.state_enter_time = current_time;
                }
                
                if (current_time - g_state.state_enter_time >= STATE_DISPLAY_DURATION) {
                    set_sys_state(STATE_LOGOUT);
                    g_keypad.current_mode=MODE_LOGOUT;
                }
                esp_task_wdt_reset();
                break;

            default:
                set_sys_state(STATE_LOGOUT);
                
                break;
        }

        update_display_state();
        handle_display();

        g_state.prev_sys_state=get_sys_state();
        
        esp_task_wdt_reset();
        
        vTaskDelay(pdMS_TO_TICKS(50));

}



 void system_task(void *pvParameters)
{
    ESP_LOGI(TAG, "System state task started");

    vTaskDelay(pdMS_TO_TICKS(100));
    esp_task_wdt_add(NULL);

    for (;;) 
    {
        system_state_update();   
    }
}