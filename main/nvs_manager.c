#include "nvs_manager.h"
#include "esp_log.h"

#define TAG_NVS "NVS_MANAGER"

#define CONFIG_LOG_MAXIMUM_LEVEL ESP_LOG_INFO

void save_to_nvs(const char* key, const char* value) {
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(key, NVS_READWRITE, &my_handle);
    ESP_ERROR_CHECK(err);
    err = nvs_set_str(my_handle, key, value);
    ESP_ERROR_CHECK(err);
    err = nvs_commit(my_handle);
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG_NVS, "Value saved to NVS successfully");
}

void read_from_nvs(const char* key, char* value, size_t max_len) {
    nvs_handle_t my_handle;
    esp_err_t err;
    size_t len;

    err = nvs_open(key, NVS_READWRITE, &my_handle);
    ESP_ERROR_CHECK(err);
    err = nvs_get_str(my_handle, key, NULL, &len);
    if(err != ESP_OK) {return;}
    if(len > max_len) {
        ESP_LOGE(TAG_NVS, "Value is too long to fit in buffer");
        return;
    }
    err = nvs_get_str(my_handle, key, value, &len);
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG_NVS, "Value read from NVS successfully");
}

void remove_from_nvs(const char* key) {
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open(key, NVS_READWRITE, &my_handle);
    ESP_ERROR_CHECK(err);
    err = nvs_erase_key(my_handle, key);
    ESP_ERROR_CHECK(err);
    err = nvs_commit(my_handle);
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG_NVS, "Value removed from NVS successfully");
}