#ifndef NVS_UTILS_H
#define NVS_UTILS_H

#include "nvs_flash.h"
#include "esp_log.h"
#include "string.h"
#include "stdio.h"



void save_counter_id(const char *counter_id);


esp_err_t read_counter_id_from_nvs(char *counter_id, size_t buffer_size) ;

void delete_counter_id(void) ;


void save_wifi_credentials(const char *ssid, const char *password, const uint8_t* bssid) ;
esp_err_t read_wifi_credentials_from_nvs(char *ssid, size_t *ssid_len_ptr, char *password, size_t *password_len_ptr, uint8_t* bssid) ;

void save_called_number(const char *number) ;
esp_err_t read_current_number_from_nvs(char *number, size_t *number_len_ptr) ;
void delete_called_number(void) ;

esp_err_t read_current_number_status_from_nvs(char *number_status, size_t buffer_size);

void save_number_status(const char *number_status);
void delete_number_status(void) ;
void save_user_pass(const char *user_pass) ;
esp_err_t read_user_pass_from_nvs(char *user_pass, size_t buffer_size) ;

void save_login_status(const char *user_pass) ;
esp_err_t read_login_status(char *user_pass, size_t buffer_size) ;

#endif