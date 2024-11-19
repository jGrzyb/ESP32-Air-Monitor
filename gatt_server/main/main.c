#include "main.h"

uint8_t ssid[32] = {0};
uint8_t password[64] = {0};

static bool wifi_init = false;

void set_wifi_config(char* value, char* ssid, char* password) {
    char *token = strtok((char *)value, " ");
    if (token != NULL) {
        strncpy((char *)ssid, token, 32 - 1);
        ssid[32 - 1] = '\0';
        token = strtok(NULL, " ");
        if (token != NULL) {
            strncpy((char *)password, token, 64 - 1);
            password[64 - 1] = '\0';
        }
    }
}

void wifi_config_to_string(char* value, char* ssid, char* password) {
    snprintf(value, 96, "%s %s", ssid, password);
}

void onBluetoothWrite(uint8_t* arrPtr, int len) {
    if (bluetoothMessage != NULL) {
        free(bluetoothMessage);
    }
    bluetoothMessage = (uint8_t*)malloc(len+1);
    memcpy(bluetoothMessage, arrPtr, len);
    bluetoothMessage[len] = '\0';
    bluetoothMessage_len = len;
    set_wifi_config((char*)bluetoothMessage, (char*)ssid, (char*)password);
    if(!wifi_init) {
        wifi_init_sta();
        wifi_init = true;
    }
}
void onBluetoothRead(esp_gatt_rsp_t* rsp) {
    rsp->attr_value.len = bluetoothMessage_len;
    memcpy(rsp->attr_value.value, bluetoothMessage, bluetoothMessage_len);
}

void onWifiConnected(void) {
    char nvs_wifi_config[96] = {0};
    wifi_config_to_string(nvs_wifi_config, (char*)ssid, (char*)password);
    save_to_nvs(WIFI_CONFIG_KEY, nvs_wifi_config);
    xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
}
void onWifiFailded(void) {
    remove_from_nvs(WIFI_CONFIG_KEY);
    esp_restart();
}

void app_main(void)
{
    esp_err_t ret;

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    char nvs_wifi_config[96] = {0};
    read_from_nvs(WIFI_CONFIG_KEY, nvs_wifi_config, sizeof(nvs_wifi_config) - 1);
    set_wifi_config(nvs_wifi_config, (char*)ssid, (char*)password);
    if(strlen(nvs_wifi_config) > 0) {
        wifi_init_sta();
    }
    else {
        bluetoothInit();
    }

    esp_rom_gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
}