#include "esp_mqtt_client.h"
#include "state_machine/state_machine.h"
#include "mac_utils/mac_utils.h"

#define TAG "MQTT"


QueueHandle_t mqtt_queue = NULL;

esp_mqtt_client_handle_t mqtt_client = NULL;



char current_number[5] = {0};

//char device_name[7] = {0};
char device_name[8] = {0};


accent_map_t accent_map[] = {
    {"á", "a"}, {"à", "a"}, {"ả", "a"}, {"ã", "a"}, {"ạ", "a"},
    {"ă", "a"}, {"ắ", "a"}, {"ằ", "a"}, {"ẳ", "a"}, {"ẵ", "a"}, {"ặ", "a"},
    {"â", "a"}, {"ấ", "a"}, {"ầ", "a"}, {"ẩ", "a"}, {"ẫ", "a"}, {"ậ", "a"},
    {"đ", "d"},
    {"é", "e"}, {"è", "e"}, {"ẻ", "e"}, {"ẽ", "e"}, {"ẹ", "e"},
    {"ê", "e"}, {"ế", "e"}, {"ề", "e"}, {"ể", "e"}, {"ễ", "e"}, {"ệ", "e"},
    {"í", "i"}, {"ì", "i"}, {"ỉ", "i"}, {"ĩ", "i"}, {"ị", "i"},
    {"ó", "o"}, {"ò", "o"}, {"ỏ", "o"}, {"õ", "o"}, {"ọ", "o"},
    {"ô", "o"}, {"ố", "o"}, {"ồ", "o"}, {"ổ", "o"}, {"ỗ", "o"}, {"ộ", "o"},
    {"ơ", "o"}, {"ớ", "o"}, {"ờ", "o"}, {"ở", "o"}, {"ỡ", "o"}, {"ợ", "o"},
    {"ú", "u"}, {"ù", "u"}, {"ủ", "u"}, {"ũ", "u"}, {"ụ", "u"},
    {"ư", "u"}, {"ứ", "u"}, {"ừ", "u"}, {"ử", "u"}, {"ữ", "u"}, {"ự", "u"},
    {"ý", "y"}, {"ỳ", "y"}, {"ỷ", "y"}, {"ỹ", "y"}, {"ỵ", "y"},

    {"Á", "A"}, {"À", "A"}, {"Ả", "A"}, {"Ã", "A"}, {"Ạ", "A"},
    {"Ă", "A"}, {"Ắ", "A"}, {"Ằ", "A"}, {"Ẳ", "A"}, {"Ẵ", "A"}, {"Ặ", "A"},
    {"Â", "A"}, {"Ấ", "A"}, {"Ầ", "A"}, {"Ẩ", "A"}, {"Ẫ", "A"}, {"Ậ", "A"},
    {"Đ", "D"},
    {"É", "E"}, {"È", "E"}, {"Ẻ", "E"}, {"Ẽ", "E"}, {"Ẹ", "E"},
    {"Ê", "E"}, {"Ế", "E"}, {"Ề", "E"}, {"Ể", "E"}, {"Ễ", "E"}, {"Ệ", "E"},
    {"Í", "I"}, {"Ì", "I"}, {"Ỉ", "I"}, {"Ĩ", "I"}, {"Ị", "I"},
    {"Ó", "O"}, {"Ò", "O"}, {"Ỏ", "O"}, {"Õ", "O"}, {"Ọ", "O"},
    {"Ô", "O"}, {"Ố", "O"}, {"Ồ", "O"}, {"Ổ", "O"}, {"Ỗ", "O"}, {"Ộ", "O"},
    {"Ơ", "O"}, {"Ớ", "O"}, {"Ờ", "O"}, {"Ở", "O"}, {"Ỡ", "O"}, {"Ợ", "O"},
    {"Ú", "U"}, {"Ù", "U"}, {"Ủ", "U"}, {"Ũ", "U"}, {"Ụ", "U"},
    {"Ư", "U"}, {"Ứ", "U"}, {"Ừ", "U"}, {"Ử", "U"}, {"Ữ", "U"}, {"Ự", "U"},
    {"Ý", "Y"}, {"Ỳ", "Y"}, {"Ỷ", "Y"}, {"Ỹ", "Y"}, {"Ỵ", "Y"},
    {NULL, NULL}
};



extern keypad_context_t g_keypad;
extern mutex_context_t g_mutex;


void custom_string(const char *input, char *output, size_t output_len) {
    if (input == NULL || output == NULL) return;
    
    memset(output, 0, output_len);
    int out_idx = 0;
    int i = 0;
    
    while (input[i] && out_idx < output_len - 1) {
        int matched = 0;
        
        for (int j = 0; accent_map[j].accent != NULL; j++) {
            int accent_len = strlen(accent_map[j].accent);
            if (strncmp(&input[i], accent_map[j].accent, accent_len) == 0) {
                int replace_len = strlen(accent_map[j].replacement);
                if (out_idx + replace_len < output_len) {
                    strcpy(&output[out_idx], accent_map[j].replacement);
                    out_idx += replace_len;
                    i += accent_len;// i+=accent_len thay vì i++; vì 1 ký tự có dấu có thể có nhiều hơn 1 phần tử char
                    matched = 1;
                    break;
                }
            }
        }
        
        if (!matched) {
            output[out_idx++] = input[i++];// ko matched->ký tự ko dấu, chỉ có 1 byte nên index input và output chỉ tăng 1 
        }
    }
    
    output[out_idx] = '\0';
}


void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            led_on();

            ESP_LOGI(TAG, "MQTT connected");
            //set_sys_state(STATE_RUNNING);
            set_mqtt_connected(true);  
            char str[128]={0};
            sprintf(str, "{\"device_id\":\"%s\",\"status\":\"online\"}",g_keypad.default_id);
            xSemaphoreTake(g_mutex.mqtt_mutex, portMAX_DELAY);

            esp_mqtt_client_publish(mqtt_client, "device/status", str, 0, 1, true);

            esp_mqtt_client_subscribe(event->client, "responsenumber", 0);
            esp_mqtt_client_subscribe(event->client, "device/list", 1);
            esp_mqtt_client_subscribe(event->client, "service/list", 1);
            esp_mqtt_client_subscribe(event->client, "feedback_status", 0);


            
           xSemaphoreGive(g_mutex.mqtt_mutex);

            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT disconnected");
            led_off();
            set_mqtt_connected(false); 
            //set_sys_state(STATE_RUNNING);
            //set_publish(false);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            {
                
            mqtt_message_t msg = {0};

            size_t topic_len = (event->topic_len < sizeof(msg.topic) - 1)? event->topic_len : (sizeof(msg.topic) - 1);
                                
            size_t payload_len = (event->data_len < sizeof(msg.payload) - 1)? event->data_len : (sizeof(msg.payload) - 1);

            memcpy(msg.topic, event->topic, topic_len);
            msg.topic[topic_len] = '\0';
            memcpy(msg.payload, event->data, payload_len);
            msg.payload[payload_len] = '\0';

            if (xQueueSend(mqtt_queue, &msg, 0) != pdTRUE) {
                ESP_LOGW(TAG, "MQTT queue full, dropped message");
              }
            }
            break;
        
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            //set_publish(false);
            led_off();
            //set_sys_state(STATE_RUNNING);
            break;    

        default:
            break;
    }
}

void mqtt_init(void) {
    ///
            static char json_msg[128]={0};
            sprintf(json_msg,"Device_id:");
            uint8_t mac[6];
            static char mac_str[128];

            if (esp_read_mac(mac, ESP_MAC_WIFI_STA) == ESP_OK) {
                snprintf(mac_str, 128,"{\"device_id\":\"%02X:%02X:%02X:%02X:%02X:%02X\",\"status\":\"offline\"}",
                        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                ESP_LOGI(TAG, "Registered device with WiFi STA MAC: %s", mac_str);
            }

        ////
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.port = 1883,
            .address.uri = "mqtt://10.10.1.21",
        },
        .credentials = {
            .username = "appuser",
            .authentication.password = "1111",
        },
        .network.timeout_ms = 60000,  

        .session = {
            //.keepalive = 60,
            .keepalive = 15,
            .disable_clean_session = false,
            .last_will.topic = "device/status",

            .last_will.msg=mac_str,
            .last_will.qos = 1,
            .last_will.retain = true,
        },


        //.task.stack_size= 8192,
        
    };

    xSemaphoreTake(g_mutex.mqtt_mutex, portMAX_DELAY);

    if (mqtt_client) {
        
        /*
        esp_mqtt_client_disconnect(mqtt_client);
        esp_mqtt_client_stop(mqtt_client);
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
        */
        
       esp_mqtt_client_reconnect(mqtt_client);
       xSemaphoreGive(g_mutex.mqtt_mutex);// quan trọng, nhớ give mqtt mutex
       return ;
    }


    set_mqtt_connected(false);
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
    xSemaphoreGive(g_mutex.mqtt_mutex);
    ESP_LOGI(TAG, "MQTT initialized");
}





void mqtt_process_task(void *pvParameters) {
    mqtt_message_t msg;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(1000);
    esp_task_wdt_add(NULL);  // 

    while (1) {
        esp_task_wdt_reset(); 

        //if ((xQueueReceive(mqtt_queue, &msg, xTicksToWait) != pdTRUE)&&(g_keypad.stop==true)) {
        if ((xQueueReceive(mqtt_queue, &msg, xTicksToWait) != pdTRUE)) {
            continue; 
        }

        ESP_LOGI(TAG, "Processing topic: %s", msg.topic);
        ESP_LOGI(TAG, "Processing topic data: %s", msg.payload);


        if (strcmp(msg.topic, "responsenumber") == 0) {
            responsenumber_topic_handler(msg);

        }
        else if (strcmp(msg.topic, "device/list") == 0) {
            device_list_handler(msg);

        }


        else if (strcmp(msg.topic, "service/list") == 0) {
            service_list_handler(msg);
        }

        else if (strcmp(msg.topic, "feedback_status") == 0) {
           

             //ESP_LOGI(TAG,"feedback_screen %s",msg.payload);

             if (strcmp(msg.payload,"connected")==0){

                char current_num[5]={0};
                size_t num_len=sizeof(current_num);

                if (read_current_number_from_nvs(current_num,&num_len)==ESP_OK){

                xSemaphoreTake(g_mutex.mqtt_mutex, portMAX_DELAY);

                esp_mqtt_client_publish(mqtt_client, "check current number", current_num, 0, 0, 0);

                xSemaphoreGive(g_mutex.mqtt_mutex);
               // led_on();
                }
             }
             else if (strcmp(msg.payload,"disconnected")==0){
                //led_off();
             }


        }

       




        vTaskDelay(pdMS_TO_TICKS(20));
        esp_task_wdt_reset();
    }
}

int extract_counter_number(const char *name) {
    if (!name) return -1;

    const char *ptr=name;
    
   
    
    if (ptr) {
        // bỏ qua khi phần tử của chuỗi không phải là số
        while (*ptr == ' ' || *ptr == ':' || *ptr == '-'||*ptr < '0' || *ptr > '9') {
            ptr++;
        }
        
        // parse số
        if (*ptr >= '0' && *ptr <= '9') {
            int num = atoi(ptr);
            if (num >= 1 && num <= 50) {
                return num;
            }
        }
    }
    
    return -1; // Không parse được
}