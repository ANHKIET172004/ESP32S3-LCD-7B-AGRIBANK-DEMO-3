#include "mac_utils.h"

#define TAG "MAC_UTILS"

char device_mac[18] = {0};           // "AA:BB:CC:DD:EE:FF"
uint8_t device_mac_raw[6] = {0};     //   {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}

char current_counter_id[3]={0};

extern keypad_context_t g_keypad;


esp_err_t read_mac_address(char *mac_str, uint8_t *mac_raw) {
    uint8_t mac[6] = {0};
    esp_err_t err;
    
    // Đọc MAC address của WiFi Station interface
    err = esp_read_mac(mac, ESP_MAC_WIFI_STA);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read MAC address: %s", esp_err_to_name(err));
        return err;
    }
    
    memcpy(mac_raw, mac, 6);
    
    // raw mảng 6 phần tử kiểu uint8_t -> 1 chuỗi 
    if (mac_str != NULL) {
        snprintf(mac_str, 18, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    
    ESP_LOGI(TAG, "MAC Address: %s", mac_str);
    return ESP_OK;
}


void counter_id_init(){
    uint8_t mac[6];
    char mac_str[18];

    if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK) {
        snprintf(mac_str, 18, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        ESP_LOGI(TAG, "WiFi STA MAC: %s", mac_str);
    }
    
    strcpy(g_keypad.default_id,mac_str);// id mặc định là mac id của thiết bị

    ////////////// id hiển thị trên LCD

    char saved_user_id[4] = {0};
    esp_err_t err = read_counter_id_from_nvs(saved_user_id, sizeof(saved_user_id));

    if (err == ESP_OK && strlen(saved_user_id) > 0) {// nếu có lưu user id trong nvs
        strncpy(g_keypad.counter_id, saved_user_id, sizeof(g_keypad.counter_id) - 1);
        g_keypad.counter_id[sizeof(g_keypad.counter_id) - 1] = '\0';
        ESP_LOGI(TAG, "Loaded user_id from NVS: %s", g_keypad.counter_id);
    } else {
        strcpy(g_keypad.counter_id, "00");// ban đầu mặc định 00
        ESP_LOGI(TAG, "No saved user_id, using default: %s", g_keypad.counter_id);
    }  

}

void read_current_counter_id(){
    uint8_t mac[6];
    char mac_str[18];

    if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK) {
        snprintf(mac_str, 18, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        ESP_LOGI(TAG, "WiFi STA MAC: %s", mac_str);
    }
    
    strcpy(g_keypad.default_id,mac_str);
    char saved_user_id[4] = {0};
    esp_err_t err = read_counter_id_from_nvs(saved_user_id, sizeof(saved_user_id));

    if (err == ESP_OK && strlen(saved_user_id) > 0) {
        strncpy(current_counter_id, saved_user_id, sizeof(g_keypad.counter_id) - 1);
        current_counter_id[sizeof(current_counter_id) - 1] = '\0';
        ESP_LOGI(TAG, "Loaded user_id from NVS: %s", g_keypad.counter_id);
    } else {
        strcpy(g_keypad.counter_id, "00");
        ESP_LOGI(TAG, "No saved user_id, using default: %s", g_keypad.counter_id);
    }  

}