#include "main.h"

uint8_t ssid[32] = {0};
uint8_t password[64] = {0};

bool was_ssid_set = false;
bool was_password_set = false;

static void if_wifi_configured() {
    if(was_ssid_set && was_password_set) {
        save_to_nvs(SSID_KEY, (char*)ssid);
        save_to_nvs(PASSWORD_KEY, (char*)password);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_restart();
    }
}

void onBluetoothWrite_a(uint8_t* arrPtr, int len) {
    memcpy(ssid, arrPtr, len);
    ssid[len] = '\0';
    was_ssid_set = true;
    if_wifi_configured();
}
void onBluetoothRead_a(esp_gatt_rsp_t* rsp) {
    rsp->attr_value.len = strlen((char*)ssid);
    memcpy(rsp->attr_value.value, ssid, strlen((char*)ssid));
}

void onBluetoothWrite_b(uint8_t* arrPtr, int len) {
    memcpy(password, arrPtr, len);
    password[len] = '\0';
    was_password_set = true;
    if_wifi_configured();
}
void onBluetoothRead_b(esp_gatt_rsp_t* rsp) {
    rsp->attr_value.len = strlen((char*)password);
    memcpy(rsp->attr_value.value, password, strlen((char*)password));
}

void onWifiConnected(void) {
    xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
}
void onWifiFailded(void) {
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

    read_from_nvs(SSID_KEY, (char*)ssid, sizeof(ssid) - 1);
    read_from_nvs(PASSWORD_KEY, (char*)password, sizeof(password) - 1);

    wifi_init_sta();
    bluetoothInit();

    esp_rom_gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
}