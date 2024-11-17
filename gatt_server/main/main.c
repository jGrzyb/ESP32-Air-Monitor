#include "main.h"

void bluetoothWrite(uint8_t* arrPtr, int len) {
    if (bluetoothMessage != NULL) {
        free(bluetoothMessage);
    }
    bluetoothMessage = (uint8_t*)malloc(len);
    memcpy(bluetoothMessage, arrPtr, len);
    bluetoothMessage_len = len;
}

void bluetoothRead(esp_gatt_rsp_t* rsp) {
    rsp->attr_value.len = bluetoothMessage_len;
    memcpy(rsp->attr_value.value, bluetoothMessage, bluetoothMessage_len);
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

    bluetoothInit();
    esp_rom_gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    
    wifi_init_sta();
    xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
}