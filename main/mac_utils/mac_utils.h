#ifndef MAC_UTILS_H
#define MAC_UTILS_H


#include "esp_mac.h"
#include "esp_log.h"
#include "stdint.h"
#include "nvs_utils/nvs_utils.h"
#include "keypad/keypad_variables.h"

esp_err_t read_mac_address(char *mac_str, uint8_t *mac_raw) ;
void user_id_init();

#endif