#include "state_machine.h"
#include "wifi/wifi.h"

#define TAG "STATE_MACHINE"

char temp_buff[17] = {0};
extern char ssid_scroll_buffer[37];


state_context_t g_state = {
    .display_state        = DISPLAY_IDLE,
    .prev_display_state   = DISPLAY_IDLE,
    .display_update_time  = 0,

    .sys_state        = STATE_INIT,
    .prev_sys_state   = STATE_INIT,
    .state_enter_time = 0,

    .system_task_handle = NULL,
    .selected_ssid = {0},
    .selected_pass = {0},
};


static void state_enter(SystemState  state);
static void state_run(SystemState  state);
static void state_exit(SystemState  state);


void handle_display(void)
{
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

    if (get_display_state() == g_state.prev_display_state) {
        if (now - g_state.display_update_time < DISPLAY_UPDATE_INTERVAL) {
            return;
        }
    }

    g_state.display_update_time = now;

    switch (get_display_state()) {

        case DISPLAY_WIFI_CONNECTING:
            lcd_send_cmd(0x0C);
            lcd_show_message("TRANG THAI WIFI:", "DANG KET NOI...");
            break;

        case DISPLAY_WIFI_SUCCESS:
            xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);
            strncpy(g_keypad.input_buffer,g_keypad.saved_input_buffer, sizeof(g_keypad.input_buffer) - 1);
            g_keypad.input_buffer[sizeof(g_keypad.input_buffer) - 1]='\0';// quan trọng
            g_keypad.buffer_index = g_keypad.saved_buffer_index;
            xSemaphoreGive(g_mutex.input_mutex);

            lcd_show_message("TRANG THAI WIFI:", "THANH CONG!");
            break;

        case DISPLAY_WIFI_ERROR:
            lcd_show_message("TRANG THAI WIFI:", "THAT BAI!");
            sprintf(g_keypad.connecting_wifi, "KHONG CO WIFI");
            break;

        case DISPLAY_MQTT_ERROR:
            lcd_show_message("GOI THAT BAI!", "KHONG KET NOI!");
            break;

        case DISPLAY_LOGOUT:
            lcd_show_message("  DA DONG QUAY!", " \"A\"  ->MO QUAY");
            break;
/*
        case DISPLAY_USER_PASS:
            lcd_show_user_pass(
                g_keypad.user_pass_index ? g_keypad.user_pass_buffer : "____"
            );
            break;

        case DISPLAY_NEW_USER_PASS:
            lcd_show_new_user_pass(
                g_keypad.user_pass_index ? g_keypad.user_pass_buffer : "____"
            );
            break;
*/
        case DISPLAY_MAIN_SCREEN:
            if (g_keypad.current_mode == MODE_NORMAL) {
                xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);
                lcd_show_main_screen(
                    g_keypad.buffer_index ? g_keypad.input_buffer : temp_buff
                );
                xSemaphoreGive(g_mutex.input_mutex);
            }
            break;

        case DISPLAY_USER_PASSWORD_ERROR:
            lcd_clear();
            lcd_put_cur(0,0);
            lcd_send_string("SAI MAT KHAU!");
            lcd_put_cur(1,0);
            lcd_send_string("THU LAI SAU!");
            break;

        case DISPLAY_CONTINUE:          lcd_show_options(); break;
     //   case DISPLAY_SAVED_WIFI:        lcd_show_saved_wifi(); break;
     //   case DISPLAY_DELET_WIFI_OPTION: lcd_show_delete_wifi_options(); break;
        case DISPLAY_MENU:              lcd_send_cmd(0x0C); lcd_show_menu(); break;
        case DISPLAY_DEVICE_LIST:       lcd_show_device_list(); break;
        case DISPLAY_SERVICE_LIST:      lcd_show_service_list(); break;
        case DISPLAY_SERVICE_POSITION:  lcd_show_position_list(); break;
     //   case DISPLAY_USER_LIST:         lcd_show_user_list(); break;
        case DISPLAY_NO_DATA:           lcd_show_message("CHUA CO DU LIEU", "THU LAI SAU"); break;
     //   case DISPLAY_SAVED_WIFI_OPTION: lcd_show_saved_wifi_option(); break;
        case DISPLAY_NO_WIFI:           lcd_show_message("KHONG CO WIFI", ""); break;
    //    case DISPLAY_AUTO_CONNECT:      lcd_show__auto_connect_options(); break;

        /*
        case DISPLAY_CONNECTING_WIFI:
            if (strlen(g_keypad.connecting_wifi) <= 16) {
                set_ssid_scroll_enable(false);
                lcd_clear();
                lcd_put_cur(0,0);
                lcd_send_string("*WIFI KET NOI:");
                lcd_put_cur(1,0);
                lcd_send_string(g_keypad.connecting_wifi);
            } else {
                sprintf(ssid_scroll_buffer, "%s   ", g_keypad.connecting_wifi);
                set_ssid_scroll_enable(true);
            }
            break;
*/
        case DISPLAY_WIFI_INPUT:
            if (g_keypad.wifi_step == 0)
                lcd_show_wifi_input(g_keypad.view);
            else
                lcd_show_wifi_pass(g_keypad.hide ? g_keypad.masked : g_keypad.view);

            lcd_put_cur(1, g_keypad.cursor_col);
            lcd_send_cmd(0x0F);
            break;

        default:
            break;
    }

    g_state.prev_display_state = get_display_state();
}


void update_display_state(void)
{
    switch (get_sys_state()) {
        
                      
        case STATE_WIFI_CONNECT:        set_display_state(DISPLAY_WIFI_CONNECTING); break;
        case STATE_WIFI_SUCCESS:        set_display_state(DISPLAY_WIFI_SUCCESS); break;
        case STATE_WIFI_ERROR:          set_display_state(DISPLAY_WIFI_ERROR); break;
        case STATE_MQTT_ERROR:          set_display_state(DISPLAY_MQTT_ERROR); break;
        case STATE_RUNNING:             set_display_state(DISPLAY_MAIN_SCREEN); break;
        case STATE_LOGOUT:              set_display_state(DISPLAY_LOGOUT); break;
        //case STATE_USER_PASS:           set_display_state(DISPLAY_USER_PASS); break;
        //case STATE_USER_PASSWORD_ERROR: set_display_state(DISPLAY_USER_PASSWORD_ERROR); break;
        //case STATE_NEW_USER_PASS:       set_display_state(DISPLAY_NEW_USER_PASS); break;
        case STATE_CONTINUE:            set_display_state(DISPLAY_CONTINUE); break;
       // case STATE_SAVED_WIFI:          set_display_state(DISPLAY_SAVED_WIFI); break;
        //case STATE_DELETE_WIFI_OPTION:  set_display_state(DISPLAY_DELET_WIFI_OPTION); break;
        case STATE_MENU:                set_display_state(DISPLAY_MENU); break;
        case STATE_DEVICE_LIST:         set_display_state(DISPLAY_DEVICE_LIST); break;
        case STATE_SERVICE_LIST:        set_display_state(DISPLAY_SERVICE_LIST); break;
        case STATE_SERVICE_POSITON:     set_display_state(DISPLAY_SERVICE_POSITION); break;
        case STATE_WIFI_INPUT:          set_display_state(DISPLAY_WIFI_INPUT); break;
        //case STATE_USER_LIST:           set_display_state(DISPLAY_USER_LIST); break;
        case STATE_NO_DATA:             set_display_state(DISPLAY_NO_DATA); break;
        //case STATE_SAVED_WIFI_OPTION:   set_display_state(DISPLAY_SAVED_WIFI_OPTION); break;
        case STATE_NO_WIFI:             set_display_state(DISPLAY_NO_WIFI); break;
        //case STATE_AUTO_CONNECT:        set_display_state(DISPLAY_AUTO_CONNECT); break;
        //case STATE__AUTO_CONNECT_WIFI:  set_display_state(DISPLAY_AUTO_CONNECT_WIFI); break;
        //case STATE_CONNECTING_WIFI:     set_display_state(DISPLAY_CONNECTING_WIFI); break;

        default:
            set_display_state(DISPLAY_IDLE);
            break;
    }
}


void system_state_update(void)
{
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    SystemState  current_state = get_sys_state();

    if (current_state != g_state.prev_sys_state||current_state==STATE_INIT) {
        state_exit(g_state.prev_sys_state);
        g_state.state_enter_time = now;
        state_enter(current_state);
    }

    state_run(current_state);

    update_display_state();
    handle_display();

    g_state.prev_sys_state = current_state;
    esp_task_wdt_reset();
    vTaskDelay(pdMS_TO_TICKS(50));
}


static void state_enter(SystemState  state)
{
    switch (state) {

        case STATE_INIT:
            wifi_config();
            break;
/*
        case STATE_USER_PASSWORD_ERROR:
            break;
            */

        default:
            break;
    }
}


static void state_run(SystemState  state)
{
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

    switch (state) {

        case STATE_WIFI_SUCCESS:
            if (now - g_state.state_enter_time >= STATE_DISPLAY_DURATION) {
                mqtt_init();
                set_sys_state(STATE_RUNNING);
            }
            break;

        case STATE_WIFI_ERROR:
        case STATE_MQTT_ERROR:
   //     case STATE_NO_DATA:
        case STATE_NO_WIFI:
            if (now - g_state.state_enter_time >= STATE_DISPLAY_DURATION) {
                g_keypad.current_mode = MODE_NORMAL;
                set_sys_state(STATE_RUNNING);
            }
            break;

            /*
        case STATE_USER_PASSWORD_ERROR:
            if (now - g_state.state_enter_time >= STATE_DISPLAY_DURATION) {
                g_keypad.current_mode = MODE_LOGOUT;
                set_sys_state(STATE_LOGOUT);
            }
            break;
*/
        default:
            break;
    }
}


static void state_exit(SystemState  state)
{
    switch (state) {

    //    case STATE_USER_PASSWORD_ERROR:
            // buzzer_off();
      //      break;

        default:
            break;
    }
}


void system_task(void *pvParameters)
{
    ESP_LOGI(TAG, "System FSM task started");
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_task_wdt_add(NULL);

    while (1) {
        system_state_update();
    }
}
