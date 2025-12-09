idf_component_register(SRCS "lcd_i2c/i2c-lcd.c" "main.c" 
                            "esp_mqtt_client/esp_mqtt_client.c"
                            "esp_mqtt_client/esp_mqtt_topic_handler.c" 
                            "esp_mqtt_client/esp_mqtt_actions.c" 
                            "keypad/keypad.c" "keypad/keypad_driver.c" 
                            "keypad/T9.c" "keypad/keypad_actions.c" 
                            "keypad/keypad_wifi_actions.c" 
                            "keypad/keypad_menu_actions.c" "keypad/keypad_cursor_actions.c" 
                            "mac_utils/mac_utils.c" "mutex/mutex.c" "state_machine/state_machine.c"
                             "state_machine/helpers.c" "wifi/wifi.c" "led/led.c"
                            "nvs_utils/nvs_utils.c" "lcd_i2c/lcd_option_handler.c"
                       PRIV_REQUIRES spi_flash
                       PRIV_REQUIRES esp_driver_gpio
                       PRIV_REQUIRES nvs_flash
                       PRIV_REQUIRES esp_wifi
                       PRIV_REQUIRES mqtt
                       PRIV_REQUIRES esp_event
                       PRIV_REQUIRES esp_netif
                       PRIV_REQUIRES esp_driver_i2c
                       PRIV_REQUIRES driver
                       PRIV_REQUIRES newlib
                       PRIV_REQUIRES json
                       PRIV_REQUIRES wifi_provisioning
                       INCLUDE_DIRS ".")
