#include "keypad_cursor_actions.h"



void lcd_render_ssid_editor() {
    lcd_lock();
    lcd_clear();

    lcd_put_cur(0, 0);
    lcd_send_string("*NHAP TEN WIFI:");
    //lcd_send_cmd(0x0F);//

    char view[17] = {0};

    if (g_keypad.wifi_len <= 16) {
        memcpy(view, g_keypad.wifi_ssid, g_keypad.wifi_len);
    } 
    else {
        memcpy(view, &g_keypad.wifi_ssid[g_keypad.ssid_window_start],16);
    }

    lcd_put_cur(1, 0);
    lcd_send_string(view);

   
    int lcd_pos = g_keypad.ssid_real_pos - g_keypad.ssid_window_start;
    if (lcd_pos < 0) lcd_pos = 0;
    if (lcd_pos > 15) lcd_pos = 15;

    lcd_put_cur(1, lcd_pos);
    lcd_send_cmd(0x0F);

    lcd_unlock();
}




void ssid_insert_char(char c) {
    if (g_keypad.wifi_len >= 32) return;

    for (int i = g_keypad.wifi_len; i >= g_keypad.ssid_real_pos; i--) {
        g_keypad.wifi_ssid[i+1] = g_keypad.wifi_ssid[i];
    }

    g_keypad.wifi_ssid[g_keypad.ssid_real_pos] = c;
    g_keypad.wifi_len++;
    g_keypad.ssid_real_pos++;

    if (g_keypad.ssid_real_pos > g_keypad.ssid_window_start + 15)
        g_keypad.ssid_window_start = g_keypad.ssid_real_pos - 15;

    lcd_render_ssid_editor();
}
    


void ssid_delete() {

    if (g_keypad.wifi_len == 0 || g_keypad.ssid_real_pos == 0)
        return;

    int del_pos = g_keypad.ssid_real_pos - 1;

    for (int i = del_pos; i < g_keypad.wifi_len; i++) {
        g_keypad.wifi_ssid[i] = g_keypad.wifi_ssid[i+1];
    }

    g_keypad.wifi_len--;
    g_keypad.ssid_real_pos--;

    if (g_keypad.ssid_real_pos < g_keypad.ssid_window_start)
        g_keypad.ssid_window_start = g_keypad.ssid_real_pos;

    lcd_render_ssid_editor();
}


void lcd_cursor_right() {
    // ko có ký tự, ko di chuyển trỏ
    if (g_keypad.wifi_len == 0) return;

    // <len, >0 thì tăng vị trí trỏ
    if (g_keypad.ssid_real_pos < g_keypad.wifi_len) {
        g_keypad.ssid_real_pos++;
    } else {
        // nếu ở cuối chuỗi thì về 0
        g_keypad.ssid_real_pos = 0;
    }

    // điều chỉnh vị trí start
    if (g_keypad.ssid_real_pos < g_keypad.ssid_window_start) {
        g_keypad.ssid_window_start = g_keypad.ssid_real_pos;
    }
    else if (g_keypad.ssid_real_pos > g_keypad.ssid_window_start + 15) {
        g_keypad.ssid_window_start = g_keypad.ssid_real_pos - 15;
    }


    //tạo str hiển thị lcd
    char lcd_str[17];
    if (g_keypad.wifi_len <= 16) {
        memcpy(lcd_str, g_keypad.input_buffer, g_keypad.wifi_len);
        lcd_str[g_keypad.wifi_len] = '\0';
    } else {
        memcpy(lcd_str, &g_keypad.input_buffer[g_keypad.ssid_window_start], 16);
        lcd_str[16] = '\0';
    }

    // hiển thị chuỗi custom lên lcd
    if (g_keypad.wifi_step == 0)
        lcd_show_wifi_input(lcd_str);
    else {
        if (g_keypad.hide) {
            char masked[17];
            int L = strlen(lcd_str);
            for (int i = 0; i < L; i++) 
            {
            if (i!=L-1){
            masked[i] = '*';
            }
            else {
            masked[i] = lcd_str[i];
            }
            }
            masked[L] = '\0';
            lcd_show_wifi_pass(masked);
        } else {
            lcd_show_wifi_pass(lcd_str);
        }
    }

    // tính vị trí trên lcd
    int cursor_col = g_keypad.ssid_real_pos - g_keypad.ssid_window_start;
    if (cursor_col < 0) cursor_col = 0;
    if (cursor_col > 15) cursor_col = 15;

    lcd_put_cur(1, cursor_col);
    lcd_send_cmd(0x0F);

}


void ssid_delete_at_cursor() {

    if (g_keypad.wifi_len == 0) return;

    // Không có ký tự phía TRƯỚC con trỏ -> không xoá được
    if (g_keypad.ssid_real_pos >= g_keypad.wifi_len) return;

    int del_pos = g_keypad.ssid_real_pos;

    // Dịch trái phần còn lại
    for (int i = del_pos; i < g_keypad.wifi_len; i++) {
        g_keypad.input_buffer[i] = g_keypad.input_buffer[i + 1];
    }

    g_keypad.wifi_len--;
    g_keypad.buffer_index--;
    g_keypad.input_buffer[g_keypad.wifi_len] = '\0';

    // Clamp con trỏ nếu vượt giới hạn
    if (g_keypad.ssid_real_pos > g_keypad.wifi_len)
        g_keypad.ssid_real_pos = g_keypad.wifi_len;

    // Reset window nếu chuỗi nhỏ hơn 16
    if (g_keypad.wifi_len <= 16) {
        g_keypad.ssid_window_start = 0;
    }
    else {
        // Điều chỉnh cửa sổ để giữ con trỏ trong view
        if (g_keypad.ssid_real_pos < g_keypad.ssid_window_start)
            g_keypad.ssid_window_start = g_keypad.ssid_real_pos;

        else if (g_keypad.ssid_real_pos > g_keypad.ssid_window_start + 15)
            g_keypad.ssid_window_start = g_keypad.ssid_real_pos - 15;
    }

    // --- TẠO VIEW LCD ---
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

    // --- HIỂN THỊ ---
    if (g_keypad.wifi_step == 0)
        lcd_show_wifi_input(view);
    else {
        if (g_keypad.hide) {
            char masked[17];
            int L = strlen(view);
            for (int i = 0; i < L-1; i++) masked[i] = '*';//<L
            masked[L]=g_keypad.input_buffer[L];
            masked[L] = '\0';
            lcd_show_wifi_pass(masked);
        } else {
            lcd_show_wifi_pass(view);
        }
    }

    // --- ĐẶT CON TRỎ ---
    int cursor_col = g_keypad.ssid_real_pos - g_keypad.ssid_window_start;
    if (cursor_col < 0) cursor_col = 0;
    if (cursor_col > 15) cursor_col = 15;

    lcd_put_cur(1, cursor_col);
    lcd_send_cmd(0x0F);
}
