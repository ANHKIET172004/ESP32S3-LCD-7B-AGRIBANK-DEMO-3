#include "keypad_cursor_actions.h"



void lcd_render_ssid_editor() {
    lcd_lock();
    lcd_clear();

    lcd_put_cur(0, 0);
    lcd_send_string("*NHAP TEN WIFI:");
    //lcd_send_cmd(0x0F);//

    char view[17] = {0};

    if (g_keypad.ssid_len <= 16) {
        memcpy(view, g_keypad.wifi_ssid, g_keypad.ssid_len);
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
    if (g_keypad.ssid_len >= 32) return;

    for (int i = g_keypad.ssid_len; i >= g_keypad.ssid_real_pos; i--) {
        g_keypad.wifi_ssid[i+1] = g_keypad.wifi_ssid[i];
    }

    g_keypad.wifi_ssid[g_keypad.ssid_real_pos] = c;
    g_keypad.ssid_len++;
    g_keypad.ssid_real_pos++;

    if (g_keypad.ssid_real_pos > g_keypad.ssid_window_start + 15)
        g_keypad.ssid_window_start = g_keypad.ssid_real_pos - 15;

    lcd_render_ssid_editor();
}
    


void ssid_delete() {

    if (g_keypad.ssid_len == 0 || g_keypad.ssid_real_pos == 0)
        return;

    int del_pos = g_keypad.ssid_real_pos - 1;

    for (int i = del_pos; i < g_keypad.ssid_len; i++) {
        g_keypad.wifi_ssid[i] = g_keypad.wifi_ssid[i+1];
    }

    g_keypad.ssid_len--;
    g_keypad.ssid_real_pos--;

    if (g_keypad.ssid_real_pos < g_keypad.ssid_window_start)
        g_keypad.ssid_window_start = g_keypad.ssid_real_pos;

    lcd_render_ssid_editor();
}


void ssid_cursor_right() {
    // Nếu chưa có ký tự nào → không cho di chuyển
    if (g_keypad.ssid_len == 0) return;

    // Tăng vị trí con trỏ (giới hạn không vượt ssid_len)
    if (g_keypad.ssid_real_pos < g_keypad.ssid_len) {
        g_keypad.ssid_real_pos++;
    } else {
        // Nếu đang ở cuối → quay về đầu (loop)
        g_keypad.ssid_real_pos = 0;
    }

    // Tự điều chỉnh cửa sổ 16 ký tự
    if (g_keypad.ssid_real_pos < g_keypad.ssid_window_start) {
        g_keypad.ssid_window_start = g_keypad.ssid_real_pos;
    }
    else if (g_keypad.ssid_real_pos > g_keypad.ssid_window_start + 15) {
        g_keypad.ssid_window_start = g_keypad.ssid_real_pos - 15;
    }

    // Sau khi di chuyển xong thì cập nhật LCD
    //render_ssid_view();
     // --- TÍNH WINDOW CHO LCD ---
    if (g_keypad.ssid_real_pos < g_keypad.ssid_window_start) {
        g_keypad.ssid_window_start = g_keypad.ssid_real_pos;
    }
    else if (g_keypad.ssid_real_pos > g_keypad.ssid_window_start + (16-1)) {
        g_keypad.ssid_window_start = g_keypad.ssid_real_pos - (16-1);
    }

    // --- TẠO VIEW 16 KÝ TỰ ---
    char view[17];
    if (g_keypad.ssid_len <= 16) {
        memcpy(view, g_keypad.input_buffer, g_keypad.ssid_len);
        view[g_keypad.ssid_len] = '\0';
    } else {
        memcpy(view, &g_keypad.input_buffer[g_keypad.ssid_window_start], 16);
        view[16] = '\0';
    }

    // --- HIỂN THỊ ---
    if (g_keypad.wifi_step == 0)
        lcd_show_wifi_input(view);
    else {
        if (g_keypad.hide) {
            char masked[17];
            int L = strlen(view);
            for (int i = 0; i < L; i++) 
            {
            if (i!=L-1){
            masked[i] = '*';
            }
            else {
            masked[i] = view[i];
            }
            }
            masked[L] = '\0';
            lcd_show_wifi_pass(masked);
        } else {
            lcd_show_wifi_pass(view);
        }
    }

    // --- TÍNH VỊ TRÍ CON TRỎ TRÊN LCD ---
    int cursor_col = g_keypad.ssid_real_pos - g_keypad.ssid_window_start;
    if (cursor_col < 0) cursor_col = 0;
    if (cursor_col > 15) cursor_col = 15;

    lcd_put_cur(1, cursor_col);
    lcd_send_cmd(0x0F);

}


void ssid_delete_at_cursor() {

    if (g_keypad.ssid_len == 0) return;

    // Không có ký tự phía TRƯỚC con trỏ -> không xoá được
    if (g_keypad.ssid_real_pos >= g_keypad.ssid_len) return;

    int del_pos = g_keypad.ssid_real_pos;

    // Dịch trái phần còn lại
    for (int i = del_pos; i < g_keypad.ssid_len; i++) {
        g_keypad.input_buffer[i] = g_keypad.input_buffer[i + 1];
    }

    g_keypad.ssid_len--;
    g_keypad.buffer_index--;
    g_keypad.input_buffer[g_keypad.ssid_len] = '\0';

    // Clamp con trỏ nếu vượt giới hạn
    if (g_keypad.ssid_real_pos > g_keypad.ssid_len)
        g_keypad.ssid_real_pos = g_keypad.ssid_len;

    // Reset window nếu chuỗi nhỏ hơn 16
    if (g_keypad.ssid_len <= 16) {
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

    if (g_keypad.ssid_len <= 16) {
        memcpy(view, g_keypad.input_buffer, g_keypad.ssid_len);
        view[g_keypad.ssid_len] = '\0';
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
