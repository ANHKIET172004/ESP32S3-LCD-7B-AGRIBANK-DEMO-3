#include "esp_mqtt_topic_handler.h"

static char* TAG="MQTT";

extern keypad_context_t g_keypad;

extern char current_number[5] ;

extern char device_name[7] ;


extern esp_mqtt_client_handle_t mqtt_client;

static void sort_device_list_by_counter(void)
{
    for (int i = 0; i < g_keypad.device_count - 1; i++) {
        for (int j = i + 1; j < g_keypad.device_count; j++) {

            //int c1 = atoi(g_keypad.device_list[i].counter_id);
            //int c2 = atoi(g_keypad.device_list[j].counter_id);

            int c1 = extract_counter_number(g_keypad.device_list[i].counter_id);
            int c2 = extract_counter_number(g_keypad.device_list[j].counter_id);

            char a[3];
            char b[3];

            sprintf(a,"%d",c1);
            sprintf(b,"%d",c2);

            //strncpy(g_keypad.device_list[i].name,a,sizeof(g_keypad.device_list[i].counter_id));
            //strncpy(g_keypad.device_list[j].name,b,sizeof(g_keypad.device_list[j].counter_id));


            if (c1 > c2) {
                // swap
                DeviceInfo  temp = g_keypad.device_list[i];
                g_keypad.device_list[i] = g_keypad.device_list[j];
                g_keypad.device_list[j] = temp;
            }
        }
    }
}


 void responsenumber_topic_handler(mqtt_message_t msg){
            cJSON *root = cJSON_Parse(msg.payload);
            if (!root) {
                ESP_LOGE(TAG, "Failed to parse responsenumber JSON");
                //continue;
                return ;
            }

            cJSON *num = cJSON_GetObjectItem(root, "number");
            cJSON *device_id = cJSON_GetObjectItem(root, "device_id");

            if (cJSON_IsString(device_id) && cJSON_IsString(num) &&
                device_id->valuestring && num->valuestring) {

                if (strcmp(device_id->valuestring, g_keypad.selected_device_id) == 0) {
                    esp_task_wdt_reset();
                    
                    if (strcmp(num->valuestring, "NoAvailable") != 0 && 
                        strlen(num->valuestring) > 0) {
                        
                        strncpy(current_number, num->valuestring, 
                               sizeof(current_number) - 1);
                        
                        if (g_keypad.buffer_index == 0 && g_keypad.current_mode == MODE_NORMAL) {
                            char display_num[DISPLAY_LINE_MAX + 1] = {0};
                            snprintf(display_num, sizeof(display_num), 
                                   "SO %s:%s", 
                                   g_keypad.pri ? "UU TIEN" : "DANG GOI",
                                   num->valuestring);
                            update_temp_buff(display_num);
                            save_number_status(g_keypad.pri ? "SO UU TIEN" : "SO DANG GOI");
                        }
                        
                        esp_task_wdt_reset();
                        save_called_number(num->valuestring);
                        xSemaphoreTake(g_mutex.mqtt_mutex, portMAX_DELAY);

                        char send_number[128]={0};

                        if(g_keypad.skip==true){
                        
                         sprintf(send_number,"{\"device_id\":\"%s\",\"number\":\"%s\",\"skip\":\"yes\"}",g_keypad.selected_device_id,num->valuestring);

                        }
                        else {
                        
                         sprintf(send_number,"{\"device_id\":\"%s\",\"number\":\"%s\",\"skip\":\"no\"}",g_keypad.selected_device_id,num->valuestring);

                        }

                        if (g_keypad.recall==false){
                         esp_mqtt_client_publish(mqtt_client, "number", send_number, 0, 0, 0);   
                        }
                        else {
                            g_keypad.recall=false;
                        }                   
                        xSemaphoreGive(g_mutex.mqtt_mutex);

                    }
                    else {
                        update_temp_buff("KHONG CO SO!");
                        vTaskDelay(pdMS_TO_TICKS(800));//

                          size_t num_len = sizeof(g_keypad.prev_number);
                        size_t status_len = 12;
                        char temp_status[12] = {0};
                                    if (read_current_number_from_nvs(g_keypad.prev_number, &num_len) == ESP_OK && 
                        strlen(g_keypad.prev_number) > 0 &&
                        read_current_number_status_from_nvs(temp_status, status_len) == ESP_OK &&
                        strlen(temp_status) > 0) {
                        
                        char display[DISPLAY_LINE_MAX + 1] = {0};
                        snprintf(display, sizeof(display), "%s:%s", temp_status, g_keypad.prev_number);
                        update_temp_buff(display);
                    } else {
                        update_temp_buff("___");
                    }            

                       // sys_state=STATE_MQTT_ERROR;
                        set_display_state(DISPLAY_MAIN_SCREEN);
                    }
                }
            }
            cJSON_Delete(root);


};

 void device_list_handler(mqtt_message_t msg){
     ESP_LOGI(TAG,"%S",msg.payload);
    cJSON *root = cJSON_Parse(msg.payload);
    if (!root) {
        ESP_LOGE(TAG, "Failed to parse device/list JSON");
        //continue;
        return ;
    }

    if (!cJSON_IsArray(root)) {
        ESP_LOGE(TAG, "device/list is not array");
        cJSON_Delete(root);
        //continue;
        return;
    }

    

    g_keypad.device_count = 0;
    cJSON *item = NULL;


    cJSON_ArrayForEach(item, root) {
        esp_task_wdt_reset();
        
        if (g_keypad.device_count >= MAX_DEVICES) break;

        cJSON *id = cJSON_GetObjectItem(item, "id");
        cJSON *name = cJSON_GetObjectItem(item, "name");

        if (cJSON_IsString(name) && name->valuestring&&cJSON_IsString(id) && id->valuestring) {
            
            // Extract số từ tên "Quay x"
            int counter_num = extract_counter_number(name->valuestring);
            //int counter_num = name->valueint;
            
            if (counter_num >= 0) {
                // Sử dụng số từ tên làm ID
                
                xSemaphoreTake(g_mutex.device_list_mutex, portMAX_DELAY);//


                snprintf(g_keypad.device_list[g_keypad.device_count].counter_id, 3, "%02d", counter_num % 100);
                ESP_LOGI(TAG, "Extracted counter ID: %d from name: %s", counter_num, name->valuestring);



                xSemaphoreGive(g_mutex.device_list_mutex);//
            } 

             else {
                ESP_LOGW(TAG, "No valid counter_id for device: %s", name->valuestring);
            }
            
                xSemaphoreTake(g_mutex.device_list_mutex, portMAX_DELAY);//
                strncpy(g_keypad.device_list[g_keypad.device_count].device_id, 
                        id->valuestring, 17);
                g_keypad.device_list[g_keypad.device_count].device_id[17] = '\0';

                xSemaphoreGive(g_mutex.device_list_mutex);//

            
            bool is_current_device = false;
            
            if (cJSON_IsString(id) && id->valuestring) {
                /////here
                counter_id_init();
                if (strcmp(id->valuestring, g_keypad.default_id) == 0) {
                    is_current_device = true;
                    ESP_LOGI(TAG, "Found current device (MAC match): %s", g_keypad.default_id);
                }
            }
            
            // Lưu id
            if (is_current_device) {
                custom_string(name->valuestring, device_name, 
                sizeof(device_name));
                /////

                /*
                strncpy(device_name,
                    g_keypad.device_list[g_keypad.device_count].counter_id
                    ,sizeof(device_name));
                  */

                /////
                strncpy(g_keypad.device_list[g_keypad.device_count].name, device_name, 15);
                char device_name_new[11]={0};
                //sprintf(device_name_new,"%s (X)",device_name);
                sprintf(device_name_new,"%s*",device_name);
                xSemaphoreTake(g_mutex.device_list_mutex, portMAX_DELAY);//
                strncpy(g_keypad.device_list[g_keypad.device_count].name, device_name_new, 15);
                g_keypad.device_list[g_keypad.device_count].name[15] = '\0';
                save_counter_id(g_keypad.device_list[g_keypad.device_count].counter_id
);
                xSemaphoreGive(g_mutex.device_list_mutex);//

            } 
            
            else {
                custom_string(name->valuestring, device_name, 
                                sizeof(device_name));
                xSemaphoreTake(g_mutex.device_list_mutex, portMAX_DELAY);//
                strncpy(g_keypad.device_list[g_keypad.device_count].name, device_name, 15);
                g_keypad.device_list[g_keypad.device_count].name[15] = '\0';
                xSemaphoreGive(g_mutex.device_list_mutex);//
            }
                
            
            g_keypad.device_count++;
        }
    }
    
    g_keypad.device_list_ready= true;
    ESP_LOGI(TAG, "Device list updated: %d devices", g_keypad.device_count);
    sort_device_list_by_counter();
    ESP_LOGI(TAG, "Sorted device list successfully");


    cJSON_Delete(root);

}

 void service_list_handler(mqtt_message_t msg){
                ESP_LOGI(TAG,"%S",msg.payload);

            cJSON *root = cJSON_Parse(msg.payload);
            if (!root) {
                ESP_LOGE(TAG, "Failed to parse service/list JSON");
                return;
                //continue;
            }

            if (!cJSON_IsArray(root)) {
                ESP_LOGE(TAG, "service/list is not array");
                cJSON_Delete(root);
                return;
                //continue;
            }

           
                g_keypad.service_count = 0;
                cJSON *item = NULL;
                char service_name[64] = {0};

                cJSON_ArrayForEach(item, root) {
                    esp_task_wdt_reset();
                    
                    if (g_keypad.service_count >= MAX_DEVICES) break;

                    cJSON *id = cJSON_GetObjectItem(item, "id");
                    cJSON *name = cJSON_GetObjectItem(item, "name");

                    if (cJSON_IsNumber(id) && cJSON_IsString(name) &&
                        id->valueint && name->valuestring) {

                        xSemaphoreTake(g_mutex.service_list_mutex, portMAX_DELAY);//
                        
                        snprintf(g_keypad.service_list[g_keypad.service_count].device_id, sizeof(g_keypad.service_list[g_keypad.service_count].device_id), "%d", id->valueint);

                        
                        custom_string(name->valuestring, service_name, 
                                        sizeof(service_name));
                                    
                        //strncpy(g_keypad.service_list[g_keypad.service_count].name, service_name, 15);
                        //g_keypad.service_list[g_keypad.service_count].name[15] = '\0';

                        strncpy(g_keypad.service_list[g_keypad.service_count].name, service_name, 64);//
                        g_keypad.service_list[g_keypad.service_count].name[64] = '\0';//

                        xSemaphoreGive(g_mutex.service_list_mutex);//
                        
                        g_keypad.service_count++;
                    }
                }
                
                g_keypad.service_list_ready = true;
                ESP_LOGI(TAG, "Service list updated: %d services", g_keypad.service_count);
            
            cJSON_Delete(root);
}
