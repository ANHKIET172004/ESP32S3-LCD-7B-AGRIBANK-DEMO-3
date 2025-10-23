idf_component_register(SRCS "i2c-lcd.c" "hello_world_main.c"
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
                       INCLUDE_DIRS ".")
