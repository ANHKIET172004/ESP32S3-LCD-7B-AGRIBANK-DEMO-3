#include "keypad_wifi_actions.h"


static void wifi_connect(){
        strncpy(g_keypad.wifi_pass, g_keypad.input_buffer, sizeof(g_keypad.wifi_pass)-1);
        g_keypad.wifi_pass[sizeof(g_keypad.wifi_pass)-1] = '\0';

        ESP_LOGI("WIFI LOG", "PASS = %s", g_keypad.wifi_pass);

        wifi_config_t wifi_config = {0};
        strncpy((char *)wifi_config.sta.ssid, g_keypad.wifi_ssid, sizeof(wifi_config.sta.ssid)-1);
        strncpy((char *)wifi_config.sta.password, g_keypad.wifi_pass, sizeof(wifi_config.sta.password)-1);
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

        esp_wifi_disconnect();
        esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
        esp_wifi_connect();
}

static void update_and_restore(uint8_t wifi_step){

    if (wifi_step==0){
        strncpy(g_keypad.wifi_ssid, g_keypad.input_buffer, sizeof(g_keypad.wifi_ssid)-1);
        g_keypad.wifi_ssid[sizeof(g_keypad.wifi_ssid)-1] = '\0';
        memset(g_keypad.input_buffer, 0, sizeof(g_keypad.input_buffer));
        g_keypad.wifi_len = 0;
        g_keypad.ssid_real_pos = 0;
        g_keypad.ssid_window_start = 0;
        g_keypad.wifi_step = 1;
        g_keypad.hide = true;  // password mặc định ẩn 
    }

    else {
        strncpy(g_keypad.input_buffer, g_keypad.saved_input_buffer, sizeof(g_keypad.input_buffer)-1);
        g_keypad.input_buffer[sizeof(g_keypad.input_buffer)-1] = '\0';
        g_keypad.wifi_len = g_keypad.saved_buffer_index;
        g_keypad.ssid_real_pos = g_keypad.wifi_len;
        if (g_keypad.wifi_len > 16)
            g_keypad.ssid_window_start = g_keypad.wifi_len - 16;
        else
            g_keypad.ssid_window_start = 0;
        g_keypad.wifi_step = 0;
        g_keypad.hide = false;
    }

    g_keypad.buffer_index = 0;
    g_keypad.wifi_step = 1;
    g_keypad.last_key = 0;
    g_keypad.key_press_count = 0;
    g_keypad.caps_lock = false;


}


void hide_wifi_input() {

    if (g_keypad.wifi_step != 1) return;   // chỉ ẩn pass

    g_keypad.hide = !g_keypad.hide;

    /*
    lcd_lock();
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("*NHAP MAT KHAU:");
    */

    xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);

    //char view[17];
    int len = g_keypad.wifi_len;

    if (len <= 16&&len>0) {
        memcpy(g_keypad. view, g_keypad.input_buffer, len);
       g_keypad. view[len] = '\0';
        g_keypad.ssid_window_start = 0;
    }
    
    else {       
        memcpy(g_keypad.view,&g_keypad.input_buffer[g_keypad.ssid_window_start],16);
        g_keypad.view[16] = '\0';

    }
        

    if (g_keypad.hide&&strlen(g_keypad.view)>0) {
      

        //char masked[17];
        int L = strlen(g_keypad.view);
        
        for (int i = 0; i < L; i++) {
        if (i!=L-1){
            g_keypad.masked[i] = '*';
        }
        else {
            g_keypad.masked[i]=g_keypad.view[i];
        }
        }
        g_keypad.masked[L] = '\0';

        /*
        lcd_put_cur(1, 0);
        lcd_send_string(masked);
        */
    }
    else {
        /*
        lcd_put_cur(1, 0);
        lcd_send_string(view);
        */

    }

    //int cursor_col = g_keypad.ssid_real_pos - g_keypad.ssid_window_start;
    g_keypad.cursor_col= g_keypad.ssid_real_pos - g_keypad.ssid_window_start;
    if (g_keypad.cursor_col < 0) g_keypad.cursor_col = 0;
    if (g_keypad.cursor_col > 15) g_keypad.cursor_col = 15;
/*
    lcd_put_cur(1, g_keypad.cursor_col);
    lcd_send_cmd(0x0F);
*/
    xSemaphoreGive(g_mutex.input_mutex);
   // lcd_unlock();
}

void capslock_input() {
    g_keypad.caps_lock = !g_keypad.caps_lock;

    xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);

   // char view[17];
   
    if (g_keypad.wifi_len <= 16) {
        memcpy( g_keypad.view, g_keypad.input_buffer, g_keypad.wifi_len);
         g_keypad.view[g_keypad.wifi_len] = '\0';
    } else {
        memcpy( g_keypad.view,
               &g_keypad.input_buffer[g_keypad.ssid_window_start],
               16);
         g_keypad.view[16] = '\0';
    }

    if (g_keypad.wifi_step == 0){
       // lcd_show_wifi_input( g_keypad.view);
    }

    else{
       // lcd_show_wifi_pass( g_keypad.view);
    }

    // CURSOR
    
    //int cursor_col = g_keypad.ssid_real_pos - g_keypad.ssid_window_start;
    g_keypad.cursor_col = g_keypad.ssid_real_pos - g_keypad.ssid_window_start;
    if (g_keypad.cursor_col < 0) g_keypad.cursor_col = 0;
    if (g_keypad.cursor_col > 15) g_keypad.cursor_col = 15;

   // lcd_put_cur(1, cursor_col);
   // lcd_send_cmd(0x0F);

    xSemaphoreGive(g_mutex.input_mutex);


}

void update_wifi_input_buffer(char key) {
    int should_replace = 0;
    char ch = get_char(key, &should_replace);

    xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);


    if (should_replace && g_keypad.ssid_real_pos > 0) {
        g_keypad.input_buffer[g_keypad.ssid_real_pos - 1] = ch;
    }

    else if (!should_replace) {
        if (g_keypad.wifi_len < WIFI_MAX_LEN) {

            // dịch phải toàn bộ phần sau con trỏ
            for (int i = g_keypad.wifi_len; i > g_keypad.ssid_real_pos; i--) {
                g_keypad.input_buffer[i] = g_keypad.input_buffer[i - 1];
            }

            g_keypad.input_buffer[g_keypad.ssid_real_pos] = ch;
            g_keypad.wifi_len++;
            g_keypad.ssid_real_pos++;
            g_keypad.input_buffer[g_keypad.wifi_len] = '\0';
        }
    }

    g_keypad.buffer_index = g_keypad.wifi_len;

    if (g_keypad.ssid_real_pos < g_keypad.ssid_window_start)
        g_keypad.ssid_window_start = g_keypad.ssid_real_pos;
    else if (g_keypad.ssid_real_pos >= g_keypad.ssid_window_start + 15)//
        g_keypad.ssid_window_start = g_keypad.ssid_real_pos - 15;

   // char view[17];

    if (g_keypad.wifi_len < 16) {//<=
        memcpy(g_keypad.view, g_keypad.input_buffer, g_keypad.wifi_len);
        g_keypad.view[g_keypad.wifi_len] = '\0';
    } else {
        memcpy(g_keypad.view,&g_keypad.input_buffer[g_keypad.ssid_window_start],16);//16
       g_keypad. view[16] = '\0';
    }
////////////////////////////
    if (g_keypad.wifi_step == 0){

    }
      //  lcd_show_wifi_input(g_keypad.view);
    else {
        if (g_keypad.hide) {
            //char masked[17];
            int L = strlen(g_keypad.view);
            if (L>0){
                for (int i = 0; i < L; i++) {
                    if (i!=L-1){
                      g_keypad.masked[i] = '*';//<L
                    }
                    else {
                        g_keypad.masked[L-1]=g_keypad.view[L-1];
                    }
                
                }
            }
                  g_keypad.masked[L] = '\0';// quan trong

  
           // lcd_show_wifi_pass(  g_keypad.masked);
        } else {
            //lcd_show_wifi_pass(g_keypad.view);
        }
    }

    //int cursor_col = g_keypad.ssid_real_pos - g_keypad.ssid_window_start;
    g_keypad.cursor_col  = g_keypad.ssid_real_pos - g_keypad.ssid_window_start;
    if (g_keypad.cursor_col < 0) g_keypad.cursor_col = 0;
    if (g_keypad.cursor_col > 15) g_keypad.cursor_col = 15;

    /*
    lcd_put_cur(1, g_keypad.cursor_col);
    lcd_send_cmd(0x0F);
    */

    xSemaphoreGive(g_mutex.input_mutex);
}

void delete_wifi_input_key() {

    xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);

    if (g_keypad.wifi_len == 0) {
        xSemaphoreGive(g_mutex.input_mutex);
        return;
    }

    if (g_keypad.ssid_real_pos == 0) {
        xSemaphoreGive(g_mutex.input_mutex);
        return;
    }

    int del_pos = g_keypad.ssid_real_pos - 1;

    // Dịch trái phần sau del_pos
    for (int i = del_pos; i < g_keypad.wifi_len; i++) {
        g_keypad.input_buffer[i] = g_keypad.input_buffer[i + 1];
    }

    g_keypad.wifi_len--;
    g_keypad.ssid_real_pos--;
    g_keypad.input_buffer[g_keypad.wifi_len] = '\0';

    if (g_keypad.wifi_len < 16) {//<=16
        g_keypad.ssid_window_start = 0;
    }
    
    /*
    else if (g_keypad.wifi_len == 15){
        g_keypad.ssid_window_start = g_keypad.wifi_len - 14;
    }
        */
        
    else {
        if (g_keypad.ssid_real_pos < g_keypad.ssid_window_start) {
            g_keypad.ssid_window_start = g_keypad.ssid_real_pos;
        }

        int max_window_start = g_keypad.wifi_len - 15;//<=16

        if (g_keypad.ssid_window_start > max_window_start) {
            g_keypad.ssid_window_start = max_window_start;
        }
    }


    //char view[17];

    if (g_keypad.wifi_len < 16) {
        memcpy(g_keypad.view, g_keypad.input_buffer, g_keypad.wifi_len);
      g_keypad.view[g_keypad.wifi_len] = '\0';
    } 
    
    else if (g_keypad.wifi_len == 16) {
        memcpy(g_keypad.view, &g_keypad.input_buffer[1], 15);
        g_keypad.view[15] = '\0';
    } 
    
    else {
        memcpy(g_keypad.view,&g_keypad.input_buffer[g_keypad.ssid_window_start],16);
        g_keypad.view[16] = '\0';
    }

//////////////////
    if (g_keypad.wifi_step == 0){}
      //  lcd_show_wifi_input(g_keypad.view);
    else {
        if (g_keypad.hide) {
            //char masked[17];
            if (strlen(g_keypad.view)>0){
                for (int i = 0; i < 16 && g_keypad.view[i]; i++)
                if (i>0){
                   g_keypad. masked[i-1] = '*';
                    g_keypad. masked[i]=g_keypad.view[i];
                    g_keypad. masked[strlen(g_keypad.view)] = '\0';// quan trọng
                }
                else {
                    g_keypad. masked[i]=g_keypad.view[i];  
                    g_keypad. masked[strlen(g_keypad.view)] = '\0';// quan trọng
                }
            }
            else {
                g_keypad. masked[strlen(g_keypad.view)]='\0';
            }
            //lcd_show_wifi_pass(masked);
        }
        else {
            //lcd_show_wifi_pass(view);
        }
    }


    //int cursor_col = g_keypad.ssid_real_pos - g_keypad.ssid_window_start;
    g_keypad. cursor_col = g_keypad.ssid_real_pos - g_keypad.ssid_window_start;
    if (g_keypad. cursor_col < 0) g_keypad. cursor_col = 0;
    if (g_keypad. cursor_col > 15) g_keypad. cursor_col = 15;

   // lcd_put_cur(1, g_keypad. cursor_col);
    //lcd_send_cmd(0x0F);

    xSemaphoreGive(g_mutex.input_mutex);
}

void enter_wifi() {

    if ((g_keypad.wifi_step == 0) && (g_keypad.wifi_len > 0)) {

        xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);
        update_and_restore(0);
        ESP_LOGI("WIFI LOG", "SSID = %s", g_keypad.wifi_ssid);
        xSemaphoreGive(g_mutex.input_mutex);

        
        //lcd_show_wifi_pass("");
        g_keypad.view[0]='\0';//
        //lcd_put_cur(1, 0);
        g_keypad.cursor_col=0;//
       // lcd_send_cmd(0x0F);


       

        return;
    }

    else if ((g_keypad.wifi_step == 1) && (g_keypad.wifi_len > 0)) {

        xSemaphoreTake(g_mutex.input_mutex, portMAX_DELAY);

        wifi_connect();

        update_and_restore(1);

        xSemaphoreGive(g_mutex.input_mutex);

        lcd_send_cmd(0x0C);   // tắt con trỏ

        memset(g_keypad.view,0,sizeof(g_keypad.view));//
        memset(g_keypad.masked,0,sizeof(g_keypad.masked));//
        g_keypad.cursor_col=0;//

        set_user_selected_wifi(true);
        set_sys_state(STATE_WIFI_CONNECT);

        return;
    }
}
