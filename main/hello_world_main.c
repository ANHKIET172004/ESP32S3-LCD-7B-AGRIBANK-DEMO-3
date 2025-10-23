

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "driver/i2c.h"
#include "i2c-lcd.h"
#include "cJSON.h"

#define I2C_MASTER_SCL_IO           GPIO_NUM_22
#define I2C_MASTER_SDA_IO           GPIO_NUM_21
#define I2C_MASTER_NUM              0
#define I2C_MASTER_FREQ_HZ          400000
#define I2C_MASTER_TX_BUF_DISABLE   0
#define I2C_MASTER_RX_BUF_DISABLE   0
#define I2C_MASTER_TIMEOUT_MS       1000

#define TAG "KEYPAD"
#define MAX_INPUT 49
#define DISPLAY_LINE_MAX 16

#define ROW1 GPIO_NUM_13
#define ROW2 GPIO_NUM_12
#define ROW3 GPIO_NUM_14
#define ROW4 GPIO_NUM_27

#define COL1 GPIO_NUM_26
#define COL2 GPIO_NUM_25
#define COL3 GPIO_NUM_33
#define COL4 GPIO_NUM_32

#define MQTT_TOPIC "esp32/keypad/number"
#define WIFI_MAX_RETRY 5
#define WIFI_ERROR_TIMEOUT_MS 3000
#define WIFI_SUCCESS_TIMEOUT_MS 2000

char saved_ssid[32] = {0};
char saved_pass[32] = {0};

char selected_ssid[32] = {0};
char selected_pass[32] = {0};
size_t ssid_len = sizeof(selected_ssid);
size_t password_len = sizeof(selected_pass);

bool hide = true;

static int wifi_retry_count = 0;
static uint32_t wifi_status_time = 0;
static bool wifi_status_displayed = false;
static bool wifi_connection_success = false;

char saved_input_buffer[MAX_INPUT + 1] = {0};
int saved_buffer_index = 0;

static bool wifi_success_displayed = false;
static bool wifi_error_displayed = false;

const char *keypad_lower[10] = {
    " 0", ".,!?'\"1-(@*#", "abc2", "def3", "ghi4",
    "jkl5", "mno6", "pqrs7", "tuv8", "wxyz9"
};

const char *keypad_upper[10] = {
    " 0", ".,!?'\"1-(@*#", "ABC2", "DEF3", "GHI4",
    "JKL5", "MNO6", "PQRS7", "TUV8", "WXYZ9"
};

const char keypad[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

static gpio_num_t rows[4] = {ROW1, ROW2, ROW3, ROW4};
static gpio_num_t cols[4] = {COL1, COL2, COL3, COL4};

//char input_buffer[MAX_INPUT + 1] = {0};
char input_buffer[5] = {0};
int buffer_index = 0;
char wifi_ssid[32] = {0};
char wifi_pass[32] = {0};
esp_mqtt_client_handle_t mqtt_client = NULL;
bool mqtt_connected = false;

char last_key = 0;
int key_press_count = 0;
uint32_t last_key_time = 0;
#define T9_TIMEOUT_MS 1000
bool caps_lock = false;

enum Mode {
    MODE_NORMAL = 0,
    MODE_WIFI_SSID = 1,
    MODE_WIFI_PASS = 2
};

int current_mode = MODE_NORMAL;
int wifi_step = 0;

bool wifi_need_mqtt_stop = false;
bool wifi_need_mqtt_start = false;
bool user_selected_wifi = false;

bool pri = false;
bool pri2=false;
char pri_num[5] = {0};
char current_number[5] = {0};

char temp_buff[17]={0};

typedef enum {
    STATE_INIT = 0,
    STATE_WIFI_CONNECT,
    STATE_WIFI_SUCCESS,
    STATE_WIFI_ERROR,
    STATE_MQTT_CONNECT,
    STATE_RUNNING,
    STATE_WIFI_CONNECTING,

    STATE_MAIN = 100
} SystemState;

static SystemState sys_state = STATE_INIT;

void lcd_show_main_screen(const char *number) {
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("ID:01");
    lcd_put_cur(1, 0);
    if (number && strlen(number) > 0) {
        char buf[DISPLAY_LINE_MAX + 1] = {0};
        strncpy(buf, number, DISPLAY_LINE_MAX);
        buf[DISPLAY_LINE_MAX] = '\0';
        lcd_send_string(buf);
    } else {
        lcd_send_string("---");
    }
}

void lcd_show_wifi_status(const char *status) {
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("TRANG THAI WIFI:");
    lcd_put_cur(1, 0);
    if (status &&( strlen(status) > 0) ) {
        char buf[DISPLAY_LINE_MAX + 1] = {0};
        strncpy(buf, status, DISPLAY_LINE_MAX);
        buf[DISPLAY_LINE_MAX] = '\0';
        lcd_send_string(buf);
    } 
    
    
    
    else {
        lcd_send_string("...");
    }
}

void lcd_show_mqtt_status(const char *status) {
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("TRANG THAI MQTT:");
    lcd_put_cur(1, 0);
    if (status &&( strlen(status) > 0) ) {
        char buf[DISPLAY_LINE_MAX + 1] = {0};
        strncpy(buf, status, DISPLAY_LINE_MAX);
        buf[DISPLAY_LINE_MAX] = '\0';
        lcd_send_string(buf);
    } 
    
    
    
    else {
        lcd_send_string("...");
    }
}

void lcd_show_wifi_input(const char *ssid) {
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("NHAP TEN WIFI:");
    lcd_put_cur(1, 0);
    if (ssid && strlen(ssid) > 0) {
        char short_ssid[DISPLAY_LINE_MAX + 1] = {0};
        strncpy(short_ssid, ssid, DISPLAY_LINE_MAX);
        short_ssid[DISPLAY_LINE_MAX] = '\0';
        lcd_send_string(short_ssid);
    } else {
        lcd_send_string("...");
    }
}

void lcd_show_wifi_pass(const char *pass) {
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("NHAP PASSWORD:");
    lcd_put_cur(1, 0);
    if (pass && strlen(pass) > 0&&hide==true) {
        int pass_len = strlen(pass);
        int show_len = pass_len;
        if (show_len > DISPLAY_LINE_MAX) show_len = DISPLAY_LINE_MAX;
        for (int i = 0; i < show_len - 1; i++) {
            lcd_send_data('*');
        }
        if (show_len > 0) {
            lcd_send_data(pass[pass_len - 1]);
        }
    } 

     else if (pass && strlen(pass) > 0&&hide==false) {
        int pass_len = strlen(pass);
        int show_len = pass_len;
        if (show_len > DISPLAY_LINE_MAX) show_len = DISPLAY_LINE_MAX;
        for (int i = 0; i < show_len ; i++) {
         
            lcd_send_data(pass[i]);
        }
    } 
    
    else {
        lcd_send_string("...");
    }
}

void lcd_show_message(const char *line1, const char *line2) {
    lcd_clear();
    lcd_put_cur(0, 0);
    if (line1) {
        char tmp[DISPLAY_LINE_MAX + 1] = {0};
        strncpy(tmp, line1, DISPLAY_LINE_MAX);
        tmp[DISPLAY_LINE_MAX] = '\0';
        lcd_send_string(tmp);
    }
    if (line2 != NULL) {
        lcd_put_cur(1, 0);
        char tmp2[DISPLAY_LINE_MAX + 1] = {0};
        strncpy(tmp2, line2, DISPLAY_LINE_MAX);
        tmp2[DISPLAY_LINE_MAX] = '\0';
        lcd_send_string(tmp2);
    }
}

static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {.clk_speed = I2C_MASTER_FREQ_HZ}
    };

    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) return err;
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode,
                              I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE, 0);
}

void save_wifi_credentials(const char *ssid, const char *password, const uint8_t* bssid)
{
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("wifi_cred", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS handle (%s)!", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "Updating Wi-Fi credentials in NVS...");
    nvs_set_str(nvs_handle, "ssid", ssid);
    nvs_set_str(nvs_handle, "password", password);

    err = nvs_commit(nvs_handle);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Wi-Fi credentials saved successfully!");
    } else {
        ESP_LOGE(TAG, "Failed to save Wi-Fi credentials: %s", esp_err_to_name(err));
    }

    nvs_close(nvs_handle);
}

esp_err_t read_wifi_credentials_from_nvs(char *ssid, size_t *ssid_len_ptr, char *password, size_t *password_len_ptr, uint8_t* bssid) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("wifi_cred", NVS_READONLY, &my_handle);
    if (err != ESP_OK) return err;

    if (ssid_len_ptr && *ssid_len_ptr > 0) {
        size_t tmp = *ssid_len_ptr;
        err = nvs_get_str(my_handle, "ssid", ssid, &tmp);
        if (err != ESP_OK) {
            nvs_close(my_handle);
            return err;
        }
        *ssid_len_ptr = tmp;
    }

    if (password_len_ptr && *password_len_ptr > 0) {
        size_t tmp2 = *password_len_ptr;
        err = nvs_get_str(my_handle, "password", password, &tmp2);
        if (err != ESP_OK) {
            nvs_close(my_handle);
            return err;
        }
        *password_len_ptr = tmp2;
    }

    nvs_close(my_handle);
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected");
            mqtt_connected = true;
            esp_mqtt_client_publish(mqtt_client, "device/status", "{\"device_id\":\"04:1A:2B:3C:4D:04\",\"status\":\"online\"}", 0, 1, true);
            esp_mqtt_client_subscribe(event->client, "responsenumber", 0);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT disconnected");
            mqtt_connected = false;
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            {
                char topic[128] = {0};
                size_t topic_len = (event->topic_len < sizeof(topic) - 1) ? event->topic_len : (sizeof(topic) - 1);
                memcpy(topic, event->topic, topic_len);
                topic[topic_len] = '\0';

                char payload[256] = {0};
                size_t payload_len = (event->data_len < sizeof(payload) - 1) ? event->data_len : (sizeof(payload) - 1);
                memcpy(payload, event->data, payload_len);
                payload[payload_len] = '\0';

                if (strcmp(topic, "responsenumber") == 0) {
                    cJSON *root = cJSON_Parse(payload);
                    if (root) {
                        cJSON *num = cJSON_GetObjectItem(root, "number");
                        cJSON *device_id = cJSON_GetObjectItem(root, "device_id");
                        const char *device_str = device_id->valuestring;
                        if (num && cJSON_IsString(num) && num->valuestring&&(strcmp(device_str,"04:1A:2B:3C:4D:04")==0)) {
                            const char *number_str = num->valuestring;
                            if (strcmp(number_str, "No number available") != 0 && strlen(number_str) > 0) {
                                strncpy(current_number, number_str, sizeof(current_number) - 1);
                                
                                if (buffer_index == 0 && current_mode == MODE_NORMAL) {
                                    if (!pri) {
                                        char display_num[DISPLAY_LINE_MAX + 1] = {0};
                                        snprintf(display_num, sizeof(display_num), "SO DANG GOI:%s", number_str);
                                        lcd_put_cur(1, 0);
                                        lcd_send_string("                ");
                                        lcd_put_cur(1, 0);
                                        lcd_send_string(display_num);
                                    } else {
                                        pri = false;
                                        char display_num[DISPLAY_LINE_MAX + 1] = {0};
                                        snprintf(display_num, sizeof(display_num), "SO UU TIEN:%s", number_str);
                                        lcd_put_cur(1, 0);
                                        lcd_send_string("                ");
                                        lcd_put_cur(1, 0);
                                        lcd_send_string(display_num);
                                    }
                                }
                                esp_mqtt_client_publish(mqtt_client, "number", number_str, 0, 1, 0);
                            }
                        }
                        cJSON_Delete(root);
                    }
                }
            }
            break;
        
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
             if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
               ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
               ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
               ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
             } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
              } else {
                 ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
               }

                lcd_show_message("THAT BAI!", "KHONG KET NOI!");
               vTaskDelay(pdMS_TO_TICKS(2000));
              if (buffer_index > 0) {
                lcd_show_main_screen(input_buffer);
              } else if (strlen(current_number) > 0) {
                lcd_show_main_screen(current_number);
            } else {
                lcd_show_main_screen("");
            }
            break;    

        default:
            break;
    }
}

void mqtt_init(void) {
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.port = 1883,
            .address.uri = "mqtt://10.10.1.27",
        },
        .credentials = {
            .username = "appuser",
            .authentication.password = "1111",
        },
        .session = {
            .keepalive = 60,
            .disable_clean_session = false,
            .last_will.topic = "device/status",
            .last_will.msg = "{\"device_id\":\"04:1A:2B:3C:4D:04\",\"status\":\"offline\"}",
            .last_will.qos = 1,
            .last_will.retain = true,
        },
    };

    if (mqtt_client) {
        mqtt_client = NULL;
        mqtt_connected = false;
    }

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
    ESP_LOGI(TAG, "MQTT initialized");
}

void mqtt_publish_number(const char *number) {
    if (!mqtt_connected) {
        ESP_LOGW(TAG, "MQTT not connected");
        lcd_show_message("GOI THAT BAI!", "KHONG KET NOI!");
        vTaskDelay(pdMS_TO_TICKS(2000));
        if (buffer_index > 0) {
            lcd_show_main_screen(input_buffer);
        } else if (strlen(current_number) > 0) {
            lcd_show_main_screen(current_number);
        } else {
            lcd_show_main_screen("");
        }
        return;
    }
    printf("GOI UU TIEN\n");
    char json_msg[128] = {0};
    snprintf(json_msg, sizeof(json_msg), "{\"device_id\":\"04:1A:2B:3C:4D:04\",\"number\":\"%s\"}", number);
    esp_mqtt_client_publish(mqtt_client, "specificnumber", json_msg, 0, 0, 0);
    ESP_LOGI(TAG, "Published number %s", number);
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi STA started");
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "WiFi connected, got IP");
        wifi_need_mqtt_start = true;
        wifi_need_mqtt_stop = false;
        wifi_retry_count = 0;
        wifi_connection_success = true;

        if (user_selected_wifi == true) {
            user_selected_wifi = false;
            save_wifi_credentials(wifi_ssid, wifi_pass, NULL);
        }
        sys_state = STATE_WIFI_SUCCESS;
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        wifi_event_sta_connected_t *ev = (wifi_event_sta_connected_t *) event_data;
        ESP_LOGI(TAG, "Connected to SSID:%s, channel:%d", ev->ssid, ev->channel);
        wifi_need_mqtt_start = true;
        wifi_need_mqtt_stop = false;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* ev = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGI(TAG, "STA disconnected, reason=%d", ev->reason);

        vTaskDelay(pdMS_TO_TICKS(500));

        wifi_need_mqtt_stop = true;
        wifi_need_mqtt_start = false;

        if (mqtt_client) {
            esp_mqtt_client_disconnect(mqtt_client);
            esp_mqtt_client_stop(mqtt_client);
        }

        if (user_selected_wifi == true) {
            wifi_retry_count++;
            ESP_LOGW(TAG, "WiFi connection failed, retry count: %d/%d", wifi_retry_count, WIFI_MAX_RETRY);
            
            if (wifi_retry_count >= WIFI_MAX_RETRY) {
                ESP_LOGE(TAG, "WiFi connection failed after %d retries", WIFI_MAX_RETRY);
                user_selected_wifi = false;
                wifi_status_displayed = true;
                sys_state = STATE_WIFI_ERROR;
                return;
            }
            esp_wifi_connect();
        } else if (user_selected_wifi == false) {
            wifi_retry_count++;
            if (wifi_retry_count < WIFI_MAX_RETRY) {
                esp_wifi_connect();
            } else {
                ESP_LOGE(TAG, "Saved WiFi connection failed after %d retries", WIFI_MAX_RETRY);
                wifi_status_displayed = true;
                sys_state = STATE_WIFI_ERROR;
            }
        }
    }
}

void wifi_init(void) {
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {0};
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "WiFi initialized");
}

void keypad_init() {
    for (int i = 0; i < 4; i++) {
        gpio_reset_pin(rows[i]);
        gpio_set_direction(rows[i], GPIO_MODE_OUTPUT);
        gpio_set_level(rows[i], 1);
    }
    for (int i = 0; i < 4; i++) {
        gpio_reset_pin(cols[i]);
        gpio_set_direction(cols[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(cols[i], GPIO_PULLUP_ONLY);
    }
}

char keypad_scan() {
    for (int row = 0; row < 4; row++) {
        for (int i = 0; i < 4; i++) {
            gpio_set_level(rows[i], (i == row) ? 0 : 1);
        }
        vTaskDelay(pdMS_TO_TICKS(5));
        for (int col = 0; col < 4; col++) {
            if (gpio_get_level(cols[col]) == 0) {
                vTaskDelay(pdMS_TO_TICKS(20));
                if (gpio_get_level(cols[col]) == 0) {
                    return keypad[row][col];
                }
            }
        }
    }
    for (int i = 0; i < 4; i++) gpio_set_level(rows[i], 1);
    return 0;
}

void wait_key_release() {
    for (int row = 0; row < 4; row++) {
        gpio_set_level(rows[row], 1);
    }
    while (1) {
        int any_pressed = 0;
        for (int col = 0; col < 4; col++) {
            if (gpio_get_level(cols[col]) == 0) {
                any_pressed = 1;
                break;
            }
        }
        if (!any_pressed) break;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

char get_char(char key, int *should_replace) {
    if (key < '0' || key > '9') {
        *should_replace = 0;
        return 0;
    }

    int digit = key - '0';
    const char **keypad_map = caps_lock ? keypad_upper : keypad_lower;
    const char *chars = keypad_map[digit];
    int len = strlen(chars);

    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

    if (key != last_key || (now - last_key_time) > T9_TIMEOUT_MS) {
        last_key = key;
        key_press_count = 1;
        last_key_time = now;
        *should_replace = 0;
        return chars[0];
    }

    key_press_count++;
    last_key_time = now;
    if (key_press_count > len) {
        key_press_count = 1;
    }

    *should_replace = 1;
    return chars[key_press_count - 1];
}

void process_key_wifi_mode(char key) {
    if (key == 'A') {
        strncpy(input_buffer, saved_input_buffer, sizeof(input_buffer) - 1);
        buffer_index = saved_buffer_index;
        
        current_mode = MODE_NORMAL;
        memset(saved_input_buffer, 0, sizeof(saved_input_buffer));
        saved_buffer_index = 0;
        wifi_step = 0;
        last_key = 0;
        key_press_count = 0;
        caps_lock = false;
        
        if (buffer_index > 0) {
            sprintf(temp_buff,"SO DANG GOI:%s",input_buffer);
            lcd_show_main_screen(input_buffer);
        } else if (strlen(current_number) > 0) {
            if (pri2==false){
            sprintf(temp_buff,"SO DANG GOI:%s",current_number);
            }
            else {
                pri2=false;
            sprintf(temp_buff,"SO UU TEIN:%s",current_number);

            }
            //lcd_show_main_screen(current_number);
            lcd_show_main_screen(temp_buff);
        } else {
            lcd_show_main_screen("");
        }
        return;
    }
    else if (key == 'D') {
        if (wifi_step == 0) {
            strncpy(wifi_ssid, input_buffer, sizeof(wifi_ssid) - 1);
            memset(input_buffer, 0, sizeof(input_buffer));
            buffer_index = 0;
            wifi_step = 1;
            last_key = 0;
            key_press_count = 0;
            caps_lock = false;
            lcd_show_wifi_pass("");
        } else {
            strncpy(wifi_pass, input_buffer, sizeof(wifi_pass) - 1);
            
            wifi_retry_count = 0;
            wifi_connection_success = false;
            
            wifi_config_t wifi_config = {0};
            strncpy((char *)wifi_config.sta.ssid, wifi_ssid, sizeof(wifi_config.sta.ssid) - 1);
            strncpy((char *)wifi_config.sta.password, wifi_pass, sizeof(wifi_config.sta.password) - 1);
            esp_wifi_disconnect();
            esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
            esp_wifi_connect();

            current_mode = MODE_NORMAL;
            
            memset(input_buffer, 0, sizeof(input_buffer));
            buffer_index = 0;
            wifi_step = 0;
            last_key = 0;
            key_press_count = 0;
            caps_lock = false;
            memset(saved_input_buffer, 0, sizeof(saved_input_buffer));
            saved_buffer_index = 0;

            user_selected_wifi = true;
            lcd_show_wifi_status("DANG KET NOI...");
            
            wifi_status_displayed = true;
            sys_state = STATE_WIFI_CONNECT;
        }
        return;
    }
    else if (key == '#') {
        if (buffer_index > 0) {
            buffer_index--;
            input_buffer[buffer_index] = '\0';
            last_key = 0;
            key_press_count = 0;

            if (wifi_step == 0) {
                lcd_show_wifi_input(input_buffer);
            } else {
                lcd_show_wifi_pass(input_buffer);
            }
        }
        return;
    }
    else if (key == '*') {
        caps_lock = !caps_lock;
        if (wifi_step == 0) {
            lcd_show_wifi_input(input_buffer);
        } else {
            lcd_show_wifi_pass(input_buffer);
        }
        return;
    }
    else if (key == 'C') {
        if ((wifi_step == 1) && (hide == true)) {
            hide = false;
            lcd_clear();
            lcd_put_cur(0, 0);
            lcd_send_string("NHAP PASSWORD:");
            lcd_put_cur(1, 0);
            if (strlen(input_buffer) > 0) {
                int pass_len = strlen(input_buffer);
                for (int i = 0; i < pass_len - 1 && i < DISPLAY_LINE_MAX; i++) {
                    lcd_send_data(input_buffer[i]);
                }
                lcd_send_data(input_buffer[pass_len - 1]);
            } else {
                lcd_send_string("...");
            }
        } else if ((wifi_step == 1) && (hide == false)) {
            hide = true;
            lcd_clear();
            lcd_put_cur(0, 0);
            lcd_send_string("NHAP PASSWORD:");
            lcd_put_cur(1, 0);
            if (strlen(input_buffer) > 0) {
                int pass_len = strlen(input_buffer);
                for (int i = 0; i < pass_len - 1 && i < DISPLAY_LINE_MAX; i++) {
                    lcd_send_data('*');
                }
                lcd_send_data(input_buffer[pass_len - 1]);
            } else {
                lcd_send_string("...");
            }
        }
    }
    else if (key >= '0' && key <= '9') {
        int should_replace = 0;
        char ch = get_char(key, &should_replace);

        if (should_replace && buffer_index > 0) {
            input_buffer[buffer_index - 1] = ch;
        } else if (!should_replace) {
            if (buffer_index < MAX_INPUT) {
                input_buffer[buffer_index] = ch;
                buffer_index++;
                input_buffer[buffer_index] = '\0';
            }
        }

        if (wifi_step == 0) {
            lcd_show_wifi_input(input_buffer);
        } else {
            lcd_show_wifi_pass(input_buffer);
        }
    }
}

void process_key_normal_mode(char key) {
    if (key >= '0' && key <= '9') {
        if (buffer_index < 4) {
            input_buffer[buffer_index] = key;
            buffer_index++;
            input_buffer[buffer_index] = '\0';
        } else {
            buffer_index = 0;
            input_buffer[buffer_index] = key;
            buffer_index++;
            input_buffer[buffer_index] = '\0';
        }
        lcd_show_main_screen(input_buffer);
        return;
    }

    if (key == 'D') {
        if (buffer_index > 0) {
            pri = true;
            pri2=true;
            mqtt_publish_number(input_buffer);
            memset(input_buffer, 0, sizeof(input_buffer));
            buffer_index = 0;
        }
        return;
    }

    if (key == 'C') {
        if (buffer_index > 0) {
            buffer_index--;
            input_buffer[buffer_index] = '\0';
            last_key = 0;
            key_press_count = 0;
            lcd_show_main_screen(input_buffer);
        }
        return;
    }

    if (key == 'A') {
        strncpy(saved_input_buffer, input_buffer, sizeof(saved_input_buffer) - 1);
        saved_buffer_index = buffer_index;
        
        current_mode = MODE_WIFI_SSID;
        memset(input_buffer, 0, sizeof(input_buffer));
        buffer_index = 0;
        wifi_step = 0;
        last_key = 0;
        key_press_count = 0;
        caps_lock = false;
        hide = true;
        lcd_show_wifi_input("");
        return;
    }

    if (key == 'B') {
        printf("GOI SO TIEP THEO\n");
        const char *json_msg = "{\"device_id\":\"04:1A:2B:3C:4D:04\",\"request\":\"number\"}";
        esp_mqtt_client_publish(mqtt_client, "requestnumber", json_msg, 0, 0, 0);
        /*
        if ((esp_mqtt_client_publish(mqtt_client, "requestnumber", json_msg, 0, 0, 0)) < 0) {
            ESP_LOGW(TAG, "MQTT not connected");
            lcd_show_message("THAT BAI!", "KHONG KET NOI!");
            vTaskDelay(pdMS_TO_TICKS(2000));
            if (buffer_index > 0) {
                lcd_show_main_screen(input_buffer);
            } else if (strlen(current_number) > 0) {
                lcd_show_main_screen(current_number);
            } else {
                lcd_show_main_screen("");
            }
        }
            */
        return;
    }

    if (key == '#') {
        const char *json_msg = "{\"device_id\":\"04:1A:2B:3C:4D:04\"}";
        printf("GOI LAI\n");
        esp_mqtt_client_publish(mqtt_client, "recallnumber", json_msg, 0, 0, 0);
/*
        if ((esp_mqtt_client_publish(mqtt_client, "recallnumber", json_msg, 0, 0, 0)) < 0) {
            ESP_LOGW(TAG, "MQTT not connected");
            lcd_show_message("THAT BAI!", "KHONG KET NOI!");
            vTaskDelay(pdMS_TO_TICKS(2000));
            if (buffer_index > 0) {
                lcd_show_main_screen(input_buffer);
            } else if (strlen(current_number) > 0) {
                lcd_show_main_screen(current_number);
            } else {
                lcd_show_main_screen("");
            }
        }
            */
        return;
    }

    if (key == '*') {
        printf("BO SO HIEN TAI\n");
        const char *json_msg = "{\"device_id\":\"04:1A:2B:3C:4D:04\"}";
        esp_mqtt_client_publish(mqtt_client, "skipnumber", json_msg, 0, 0, 0);
        /*
        if ((esp_mqtt_client_publish(mqtt_client, "skipnumber", json_msg, 0, 0, 0)) < 0) {
            ESP_LOGW(TAG, "MQTT not connected");
            lcd_show_message("THAT BAI!", "KHONG KET NOI!");
            vTaskDelay(pdMS_TO_TICKS(2000));
            if (buffer_index > 0) {
                lcd_show_main_screen(input_buffer);
            } else if (strlen(current_number) > 0) {
                lcd_show_main_screen(current_number);
            } else {
                lcd_show_main_screen("");
            }
        }
            */
        return;
    }
}

void process_key(char key) {
    if (current_mode == MODE_NORMAL) {
        process_key_normal_mode(key);
    } else if (current_mode == MODE_WIFI_SSID || current_mode == MODE_WIFI_PASS) {
        process_key_wifi_mode(key);
    }
}

void keypad_task(void *param) {
    char key;
    while (1) {
        key = keypad_scan();
        if (key != 0) {
            printf("Key pressed: %c\n", key);
            process_key(key);
            wait_key_release();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void app_main(void) {
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    lcd_init();
    lcd_clear();

    lcd_put_cur(0, 0);
    lcd_send_string("    XIN CHAO");
    lcd_put_cur(1, 0);
    lcd_send_string("DANG KHOI DONG..");

    vTaskDelay(pdMS_TO_TICKS(1500));
    ESP_LOGI(TAG, "Starting keypad MQTT application");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    esp_netif_init();
    esp_event_loop_create_default();

    keypad_init();
    wifi_init();

    xTaskCreate(keypad_task, "keypad_task", 4 * 1024, NULL, 6, NULL);

    while (1) {
        switch (sys_state) {
            case STATE_INIT:
            {
                ///lcd_show_message("Khoi dong...", "");
                esp_err_t err = read_wifi_credentials_from_nvs(saved_ssid, &ssid_len, saved_pass, &password_len, NULL);
                if (err == ESP_OK && strlen(saved_ssid) > 0) {
                    ESP_LOGI(TAG, "Have a saved wifi network in NVS: %s", saved_ssid);
                    lcd_show_wifi_status("DANG KET NOI...");
                    vTaskDelay(pdMS_TO_TICKS(800));
                    
                    wifi_retry_count = 0;
                    wifi_connection_success = false;
                    
                    wifi_config_t wifi_config = {0};
                    strncpy((char *)wifi_config.sta.ssid, saved_ssid, sizeof(wifi_config.sta.ssid) - 1);
                    strncpy((char *)wifi_config.sta.password, saved_pass, sizeof(wifi_config.sta.password) - 1);
                    esp_wifi_disconnect();
                    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
                    esp_wifi_stop();
                    esp_wifi_start();
                    esp_wifi_connect();
                    sys_state = STATE_WIFI_CONNECT;
                } else {
                    lcd_show_message("CHUA LUU WIFI", "");
                    vTaskDelay(pdMS_TO_TICKS(1500));
                    lcd_show_main_screen("");
                    sys_state = STATE_RUNNING;
                }
                break;
            }

            case STATE_WIFI_CONNECT:
                if (wifi_need_mqtt_start) {
                    sys_state = STATE_MQTT_CONNECT;
                }
                break;

            case STATE_WIFI_SUCCESS:
            {
                if (!wifi_success_displayed) {
                    lcd_show_wifi_status("THANH CONG");
                    wifi_status_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    wifi_success_displayed = true;
                }
                
                uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
                if ((current_time - wifi_status_time) > WIFI_SUCCESS_TIMEOUT_MS) {
                    mqtt_init();
                    wifi_success_displayed = false;
                    sys_state = STATE_MQTT_CONNECT;
                }
                break;
            }

            case STATE_MQTT_CONNECT:
            {
                if (!wifi_success_displayed) {
                    lcd_show_mqtt_status("DANG KET NOI...");
                    wifi_status_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    wifi_success_displayed = true;
                }
                
                if (mqtt_connected) {
                    memset(input_buffer, 0, sizeof(input_buffer));
                    buffer_index = 0;
                    memset(saved_input_buffer, 0, sizeof(saved_input_buffer));
                    saved_buffer_index = 0;
                    current_number[0] = '\0';
                    lcd_clear();
                    lcd_show_main_screen("");
                    wifi_success_displayed = false;
                    sys_state = STATE_RUNNING;
                }
                break;
            }

            case STATE_RUNNING:
                break;

            case STATE_WIFI_ERROR:
            {
                if (!wifi_error_displayed) {
                    lcd_show_wifi_status("KET NOI THAT BAI");
                    wifi_status_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
                    wifi_error_displayed = true;
                }
                
                uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
                if ((current_time - wifi_status_time) > WIFI_ERROR_TIMEOUT_MS) {
                    lcd_clear();
                    lcd_show_main_screen("");
                    wifi_error_displayed = false;
                    sys_state = STATE_RUNNING;
                }
                break;
            }

            case STATE_WIFI_CONNECTING:
                lcd_clear();
                lcd_show_wifi_status("DANG KET NOI...");
                sys_state = STATE_WIFI_CONNECT;
                break;

            default:
                sys_state = STATE_INIT;
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

