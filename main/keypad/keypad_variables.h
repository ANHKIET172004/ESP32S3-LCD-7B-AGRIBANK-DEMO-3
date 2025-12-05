// keypad_context.h
#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_DEVICES 10

typedef struct {
    char device_id[18];// mac id
    char name[16];
    //char name[64];
    char user_id[3];
} DeviceInfo;

typedef struct {
    char device_id[18];// mac id
    //char name[16];
    char name[64];
    char user_id[3];
} ServiceInfo;


typedef struct {
    char        input_buffer[64];//17
    uint8_t         buffer_index;
    char        saved_input_buffer[64];//17
    int         saved_buffer_index;
    char        prev_number[5];
    //char        wifi_ssid[17];
    char        wifi_pass[64];//17
    char        saved_ssid[32] ;//17
    char        saved_pass[64] ;//17
    int         current_mode;
    uint8_t         wifi_step;
    bool        caps_lock;
    char        last_key;
    int         key_press_count;

    uint32_t    last_key_time;
    uint8_t     device_count;
    uint8_t     selected_index;
    uint8_t     service_count;
    int8_t      selected_index2;
    bool        in_selection_mode;

    bool        selected_positon;

    char        selected_device_name[18];
    char        selected_device_id[18];

    char        selected_service_name[64];//18
    char        selected_service_id[64];//18

    char        default_id[18];

    uint8_t     menu_selection; 
    bool        skip;
    bool        recall;
    bool        pri;
    char        temp_selected_device_name[18];
    char        temp_selected_device_id[18];
    bool        hide;
    uint64_t    last_t_press_time;
    uint64_t    last_a_press_time ; 
    uint64_t    last_b_press_time;   // Lưu thời gian nhấn B gần nhất (microsecond)

    uint64_t    last_s_press_time ; 
    uint64_t    debounce_interval_us;
    uint8_t     positon_flag;
    bool        device_list_ready ;
    char        user_id[3];
    bool        service_list_ready ;
    bool        switch_device;
    char        prev_number_status[12];
    DeviceInfo  device_list[MAX_DEVICES];

    ServiceInfo  service_list[MAX_DEVICES];
    bool         stop;
    int          ssid_real_pos;       
    int          ssid_window_start;   
    int          ssid_len;            
    char         wifi_ssid[32]; 
    char         view[17];
    int          len;    






} keypad_context_t;

extern keypad_context_t g_keypad;