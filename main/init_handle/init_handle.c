#include "init_handle.h"
#include "esp_log.h"
#include "nvs_flash.h"   // Để có thể erase nếu cần (optional)

static const char *TAG = "INIT_HANDLE";
static const uint8_t MAX_RETRY = 5;

// hàm mở namespace 
static esp_err_t open_nvs_namespace(nvs_handle_t *handle, nvs_open_mode_t mode) {
    esp_err_t err = nvs_open("ESP_RESTART", mode, handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed: %s (mode: %s)", 
                 esp_err_to_name(err), mode == NVS_READWRITE ? "READWRITE" : "READONLY");
    }
    return err;
}

esp_err_t save_retry(uint8_t x) {
    nvs_handle_t handle;
    esp_err_t err = open_nvs_namespace(&handle, NVS_READWRITE);
    if (err != ESP_OK) return err;

    char key[12];  // size an toàn cho chuỗi bao gồm "retry" + uint8 + null
    snprintf(key, sizeof(key), "retry%u", x);

    uint8_t cnt = 0;
    err = nvs_get_u8(handle, key, &cnt);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        cnt = 0;  // OK, sẽ set thành 1
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_get_u8 failed for %s: %s", key, esp_err_to_name(err));
        nvs_close(handle);
        return err;
    }

    cnt++;
    err = nvs_set_u8(handle, key, cnt);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_u8 failed for %s (value=%u): %s", key, cnt, esp_err_to_name(err));
        nvs_close(handle);
        return err;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Saved retry for init %u → count = %u", x, cnt);
    }

    nvs_close(handle);
    return err;
}

uint8_t read_retry(uint8_t x) {
    nvs_handle_t handle;
    esp_err_t err = open_nvs_namespace(&handle, NVS_READONLY);
    if (err != ESP_OK) return 0;

    char key[12];
    snprintf(key, sizeof(key), "retry%u", x);

    uint8_t cnt = 0;
    err = nvs_get_u8(handle, key, &cnt);
    nvs_close(handle);

    if (err == ESP_OK) {
        return cnt;
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        return 0;
    } else {
        ESP_LOGE(TAG, "read_retry failed for %s: %s", key, esp_err_to_name(err));
        return 0;
    }
}

void reset_retry(uint8_t x) {
    nvs_handle_t handle;
    esp_err_t err = open_nvs_namespace(&handle, NVS_READWRITE);
    if (err != ESP_OK) return;

    char key[12];
    snprintf(key, sizeof(key), "retry%u", x);

    err = nvs_set_u8(handle, key, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "reset_retry nvs_set_u8 failed: %s", esp_err_to_name(err));
        nvs_close(handle);
        return;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "reset_retry commit failed: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Reset retry counter for init %u", x);
    }

    nvs_close(handle);
}

void init_fail_handle(uint8_t x) {
    uint8_t counter = read_retry(x);  // local, không dùng global

    if (counter < MAX_RETRY) {
        ESP_LOGE(TAG, "INIT %u FAILED → retry %u/%u", x, counter + 1, MAX_RETRY);
        esp_err_t err = save_retry(x);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "save_retry failed → still restarting anyway");
        }
        vTaskDelay(pdMS_TO_TICKS(1500));  // tăng delay nhẹ để flash có thời gian
        esp_restart();
    } else {
        ESP_LOGE(TAG, "INIT %u FAILED AFTER %u RETRIES → HALTING SYSTEM", x, MAX_RETRY);
        // Optional: reset counter để lần sau có thể thử lại (nếu reset bằng tay)
        // reset_retry(x);

        while (1) {
            vTaskDelay(pdMS_TO_TICKS(5000));  // log ít hơn để tiết kiệm CPU
            ESP_LOGE(TAG, "SYSTEM HALTED due to repeated init failure (init ID: %u)", x);
        }
    }
}