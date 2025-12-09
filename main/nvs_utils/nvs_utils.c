#include "nvs_utils/nvs_utils.h"

#define TAG "NVS_UTILS"

void save_counter_id(const char *counter_id) {
    if (!counter_id || strlen(counter_id) == 0) {
        ESP_LOGW(TAG, "Invalid counter_id to save");
        return;
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("user_config", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS handle for counter_id (%s)!", esp_err_to_name(err));
        return;
    }

    err = nvs_set_str(nvs_handle, "counter_id", counter_id);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set counter_id: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return;
    }

    err = nvs_commit(nvs_handle);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "User ID saved successfully: %s", counter_id);
    } else {
        ESP_LOGE(TAG, "Failed to commit counter_id: %s", esp_err_to_name(err));
    }
    
    nvs_close(nvs_handle);
}


esp_err_t read_counter_id_from_nvs(char *counter_id, size_t buffer_size) {
    if (!counter_id || buffer_size == 0) {
        ESP_LOGE(TAG, "Invalid buffer for read_counter_id_from_nvs");
        return ESP_ERR_INVALID_ARG;
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("user_config", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to open NVS handle for counter_id: %s", esp_err_to_name(err));
        memset(counter_id, 0, buffer_size);
        return err;
    }

    size_t required_size = buffer_size;
    err = nvs_get_str(nvs_handle, "counter_id", counter_id, &required_size);
    
    if (err != ESP_OK) {
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(TAG, "User ID not found in NVS");
        } else {
            ESP_LOGE(TAG, "Failed to read counter_id: %s", esp_err_to_name(err));
        }
        memset(counter_id, 0, buffer_size);
    } else {
        ESP_LOGI(TAG, "Read user ID successfully: %s", counter_id);
    }
    
    nvs_close(nvs_handle);
    return err;
}


void delete_counter_id(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("user_config", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS handle (%s)!", esp_err_to_name(err));
        return;
    }

    err = nvs_erase_key(nvs_handle, "counter_id");
    if (err == ESP_OK) {
        err = nvs_commit(nvs_handle);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "User ID deleted successfully!");
        } else {
            ESP_LOGE(TAG, "Failed to commit counter_id deletion: %s", esp_err_to_name(err));
        }
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "User ID not found, nothing to delete");
    } else {
        ESP_LOGE(TAG, "Failed to delete counter_id: %s", esp_err_to_name(err));
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

bool wifi_credentials_changed(const char *new_ssid, const char *new_password)
{
   char old_ssid[17]={0};
   char old_pass[17]={0};
   size_t old_ssid_len=sizeof(old_ssid);
   size_t old_pass_len=sizeof (old_pass);


   esp_err_t err=read_wifi_credentials_from_nvs(old_ssid,&old_ssid_len,old_pass,&old_pass_len,NULL);

   if (err!=ESP_OK) return true;

   if (strncmp(new_ssid,old_ssid,16)!=0) return true;
   if (strncmp(new_password,old_pass,16)!=0) return true;

   return false;
}


void save_wifi_credentials(const char *ssid, const char *password, const uint8_t* bssid) {

    if (wifi_credentials_changed(ssid,password)==false){
         ESP_LOGI(TAG, "WIFI_CRED NO CHANGE, SKIP SAVING");
        return;
    }
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("wifi_cred", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS handle (%s)!", esp_err_to_name(err));
        return;
    }

    
    err = nvs_set_str(nvs_handle, "ssid", ssid);
    if (err!= ESP_OK) goto fail;
    err = nvs_set_str(nvs_handle, "password", password);
    if (err!=ESP_OK) goto fail;
    err = nvs_commit(nvs_handle);
    if (err!=ESP_OK) goto fail;

    ESP_LOGI(TAG, "Wi-Fi credentials saved successfully!");

    nvs_close(nvs_handle);
    return;

    fail:
    ESP_LOGE(TAG, "Failed to save Wi-Fi credentials: %s", esp_err_to_name(err));
    nvs_close(nvs_handle);
}


void save_called_number(const char *number) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("number_cred", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS handle (%s)!", esp_err_to_name(err));
        return;
    }
    nvs_set_str(nvs_handle, "current_number", number);
    err = nvs_commit(nvs_handle);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "current number saved successfully!");
    } else {
        ESP_LOGE(TAG, "Failed to save current number: %s", esp_err_to_name(err));
    }
    nvs_close(nvs_handle);
}

esp_err_t read_current_number_from_nvs(char *number, size_t *number_len_ptr) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("number_cred", NVS_READONLY, &my_handle);
    if (err != ESP_OK) return err;

    if (number_len_ptr && *number_len_ptr > 0) {
        size_t tmp = *number_len_ptr;
        err = nvs_get_str(my_handle, "current_number", number, &tmp);
        if (err != ESP_OK) {
            nvs_close(my_handle);
            return err;
        }
        *number_len_ptr = tmp;
    }
    nvs_close(my_handle);
    return ESP_OK;
}

void delete_called_number(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("number_cred", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS handle (%s)!", esp_err_to_name(err));
        return;
    }
    err = nvs_erase_key(nvs_handle, "current_number");
    if (err == ESP_OK) {
        err = nvs_commit(nvs_handle);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "current_number deleted successfully!");
        }
    }
    nvs_close(nvs_handle);
}

esp_err_t read_current_number_status_from_nvs(char *number_status, size_t buffer_size) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("number_cred", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to open NVS handle: %s", esp_err_to_name(err));
        memset(number_status, 0, buffer_size);
        return err;
    }

    size_t required_size = buffer_size;
    err = nvs_get_str(my_handle, "number_status", number_status, &required_size);
    
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to read status: %s", esp_err_to_name(err));
        memset(number_status, 0, buffer_size);
    } else {
        ESP_LOGI(TAG, "Read status successfully: %s", number_status);
    }
    nvs_close(my_handle);
    return err;
}

void save_number_status(const char *number_status) {
    if (!number_status || strlen(number_status) == 0) {
        ESP_LOGW(TAG, "Invalid status to save");
        return;
    }

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("number_cred", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS handle (%s)!", esp_err_to_name(err));
        return;
    }

    nvs_set_str(nvs_handle, "number_status", number_status);
    err = nvs_commit(nvs_handle);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Status saved successfully: %s", number_status);
    } else {
        ESP_LOGE(TAG, "Failed to save status: %s", esp_err_to_name(err));
    }
    nvs_close(nvs_handle);
}

void delete_number_status(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("number_cred", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS handle (%s)!", esp_err_to_name(err));
        return;
    }

    err = nvs_erase_key(nvs_handle, "number_status");
    if (err == ESP_OK) {
        err = nvs_commit(nvs_handle);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "Status deleted successfully!");
        }
    }
    nvs_close(nvs_handle);
}
